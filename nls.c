#include <nl_types.h>
#include <stdio.h>

#include "nls.h"

nl_catd nls_catalog = (nl_catd)-1;

void nls_catalog_set(const char *catalog){
    if (nls_catalog != (nl_catd)-1){
        catclose(nls_catalog);
    }
    nls_catalog = catopen(catalog, NL_CAT_LOCALE);
}

void nls_catalog_close(){
    if (nls_catalog != (nl_catd)-1){
        catclose(nls_catalog);
        nls_catalog = (nl_catd)-1;
    }
}

void nls_message(int message_number, const char *fallback){
    if (nls_catalog == (nl_catd)-1){
        fprintf(stderr, "%s\n", fallback);
        return;
    }
    fprintf(stderr, "%s: %s\n", catgets(nls_catalog, 1, 1, fallback), catgets(nls_catalog, 1, message_number, fallback));
}