/* Copyright (C) 2020  C. McEnroe <june@causal.agency>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <err.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capsicum.h>
#include <sys/types.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include <kcgi.h>
#include <kcgihtml.h>

static int cwd = -1;

static const struct kvalid Key = { NULL, "file" };

static enum kcgi_err head(struct kreq *req, enum khttp http, enum kmime mime) {
	return khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[http])
		|| khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[mime]);
}

static enum kcgi_err fail(struct kreq *req, enum khttp http) {
	return head(req, http, KMIME_TEXT_PLAIN)
		|| khttp_body(req)
		|| khttp_printf(req, "%s\n", khttps[http]);
}

static enum kcgi_err handle(struct kreq *req) {
	if (req->page) return fail(req, KHTTP_404);

	if (req->method == KMETHOD_GET) {
		struct khtmlreq html;
		struct khtmlreq *h = &html;
		return head(req, KHTTP_200, KMIME_TEXT_HTML)
			|| khttp_body(req)
			|| khtml_open(h, req, 0)
			|| khtml_elem(h, KELEM_DOCTYPE)
			|| khtml_elem(h, KELEM_TITLE)
			|| khtml_puts(h, "Upload")
			|| khtml_closeelem(h, 1)
			|| khtml_attr(
				h, KELEM_FORM,
				KATTR_METHOD, "post",
				KATTR_ACTION, "",
				KATTR_ENCTYPE, "multipart/form-data",
				KATTR__MAX
			)
			|| khtml_attr(
				h, KELEM_INPUT,
				KATTR_TYPE, "file",
				KATTR_NAME, Key.name,
				KATTR__MAX
			)
			|| khtml_attr(
				h, KELEM_INPUT,
				KATTR_TYPE, "submit",
				KATTR_VALUE, "Upload",
				KATTR__MAX
			)
			|| khtml_close(h);

	} else if (req->method == KMETHOD_POST) {
		struct kpair *field = req->fieldmap[0];
		if (!field || !field->valsz) return fail(req, KHTTP_400);

		char name[256];
		const char *ext = strrchr(field->file, '.');
		if (!ext) ext = "";
		snprintf(
			name, sizeof(name), "%jx%08x%s",
			(intmax_t)time(NULL), arc4random(), ext
		);

		int fd = openat(cwd, name, O_CREAT | O_EXCL | O_WRONLY, 0644);
		if (fd < 0) {
			warn("openat");
			return fail(req, KHTTP_507);
		}
		ssize_t len = write(fd, field->val, field->valsz);
		int error = close(fd);
		if (len < 0 || error) {
			warn("write");
			return fail(req, KHTTP_507);
		}

		return head(req, KHTTP_303, KMIME_TEXT_PLAIN)
			|| khttp_head(req, kresps[KRESP_LOCATION], "/%s", name)
			|| khttp_body(req)
			|| khttp_puts(req, name);

	} else {
		return fail(req, KHTTP_405);
	}
}

static void sandbox(void) {
	cwd = open(".", O_DIRECTORY);
	if (cwd < 0) err(EX_CONFIG, ".");

	int error = cap_enter();
	if (error) err(EX_OSERR, "cap_enter");

	cap_rights_t rights;
	cap_rights_init(&rights, CAP_LOOKUP, CAP_CREATE, CAP_PWRITE);
	error = cap_rights_limit(cwd, &rights);
	if (error) err(EX_OSERR, "cap_rights_limit");
}

int main(void) {
	const char *page = "up";
	if (khttp_fcgi_test()) {
		struct kfcgi *fcgi;
		enum kcgi_err error = khttp_fcgi_init(&fcgi, &Key, 1, &page, 1, 0);
		if (error) errx(EX_CONFIG, "khttp_fcgi_init: %s", kcgi_strerror(error));
		sandbox();
		for (
			struct kreq req;
			KCGI_OK == (error = khttp_fcgi_parse(fcgi, &req));
			khttp_free(&req)
		) {
			error = handle(&req);
			if (error && error != KCGI_HUP) break;
		}
		if (error != KCGI_EXIT) {
			errx(EX_PROTOCOL, "khttp_fcgi_parse: %s", kcgi_strerror(error));
		}
		khttp_fcgi_free(fcgi);
	} else {
		struct kreq req;
		enum kcgi_err error = khttp_parse(&req, &Key, 1, &page, 1, 0);
		if (error) errx(EX_PROTOCOL, "khttp_parse: %s", kcgi_strerror(error));
		error = handle(&req);
		if (error) errx(EX_PROTOCOL, "%s", kcgi_strerror(error));
		khttp_free(&req);
	}
}
