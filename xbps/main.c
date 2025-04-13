#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xbps.h>

#define UNUSED __attribute__((__unused__))

int do_post(char const *const url) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  struct curl_slist *slist1 = NULL;

  if (curl) {
    slist1 = curl_slist_append(
        slist1, "Content-Type: application/json; charset: utf-8");
    slist1 = curl_slist_append(slist1, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
                     (long)strlen("name=daniel&project=curl"));
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(slist1);
  }

  curl_global_cleanup();

  return 0;
}

int list_pkgs_in_dict(struct xbps_handle *xhp UNUSED, xbps_object_t obj,
                      const char *key UNUSED, void *arg UNUSED,
                      bool *loop_done UNUSED) {
  char *pkgver = NULL;

  xbps_dictionary_get_cstring(obj, "pkgver", &pkgver);

  char *name, *version;

  xbps_dictionary_get_cstring(obj, "pkgname", &name);
  xbps_dictionary_get_cstring(obj, "pkgver", &version);

  char *index = strrchr(pkgver, '-');

  char *version_string = strdup(index + 1);

  printf("%s--%s\n", name, version_string);

  return 0;
}

static int list_pkgs_pkgdb(struct xbps_handle *xh) {
  return xbps_pkgdb_foreach_cb(xh, list_pkgs_in_dict, NULL);
}

int main(int argc, char **argv) {
  int rv;

  struct xbps_handle xh;
  memset(&xh, 0, sizeof(xh));

  if ((rv = xbps_init(&xh)) != 0) {
    fprintf(stderr, "xbps_init failed\n");
    return rv;
  }

  list_pkgs_pkgdb(&xh);

  xbps_end(&xh);

  return 0;
}
