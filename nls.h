#ifndef NLS_H
#define NLS_H

void nls_catalog_set(const char *catalog);
void nls_catalog_close();
void nls_message(const unsigned short message_number, const char *fallback);

#endif