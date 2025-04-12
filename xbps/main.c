#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xbps.h>

#define UNUSED __attribute__((__unused__))

struct list_pkgver_cb {
  unsigned int pkgver_len;
  unsigned int maxcols;
  char *linebuf;
};

int list_pkgs_in_dict(struct xbps_handle *xhp UNUSED, xbps_object_t obj,
                      const char *key UNUSED, void *arg,
                      bool *loop_done UNUSED) {
  struct list_pkgver_cb *lpc = arg;
  const char *pkgver = NULL, *short_desc = NULL, *state_str = NULL;
  unsigned int len;
  pkg_state_t state;

  xbps_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver);
  xbps_dictionary_get_cstring_nocopy(obj, "short_desc", &short_desc);
  if (!pkgver || !short_desc)
    return EINVAL;

  xbps_pkg_state_dictionary(obj, &state);

  if (state == XBPS_PKG_STATE_INSTALLED)
    state_str = "ii";
  else if (state == XBPS_PKG_STATE_UNPACKED)
    state_str = "uu";
  else if (state == XBPS_PKG_STATE_HALF_REMOVED)
    state_str = "hr";
  else
    state_str = "??";

  if (lpc->linebuf == NULL) {
    printf("%s %-*s %s\n", state_str, lpc->pkgver_len, pkgver, short_desc);
    return 0;
  }

  len = snprintf(lpc->linebuf, lpc->maxcols, "%s %-*s %s", state_str,
                 lpc->pkgver_len, pkgver, short_desc);
  /* add ellipsis if the line was truncated */
  if (len >= lpc->maxcols && lpc->maxcols > 4) {
    for (unsigned int j = 0; j < 3; j++)
      lpc->linebuf[lpc->maxcols - j - 1] = '.';
    lpc->linebuf[lpc->maxcols] = '\0';
  }
  puts(lpc->linebuf);
  printf("%s\n", lpc->linebuf);

  return 0;
}

static int list_pkgs_pkgdb(struct xbps_handle *xh) {
  struct list_pkgver_cb lpc;

  return xbps_pkgdb_foreach_cb(xh, list_pkgs_in_dict, &lpc);
}

int main(int argc, char **argv) {
  const char *rootdir, *cachedir, *confdir;

  int rv;

  struct xbps_handle xh;
  memset(&xh, 0, sizeof(xh));

  if (rootdir) {
    xbps_strlcpy(xh.rootdir, rootdir, sizeof(xh.rootdir));
  }
  if (cachedir) {
    xbps_strlcpy(xh.cachedir, cachedir, sizeof(xh.cachedir));
  }
  if (confdir) {
    xbps_strlcpy(xh.confdir, confdir, sizeof(xh.confdir));
  }

  if ((rv = xbps_init(&xh)) != 0) {
    fprintf(stderr, "xbps_init failed\n");
    return rv;
  }

  list_pkgs_pkgdb(&xh);

  xbps_end(&xh);

  return 0;
}
