![Fannst Banner](../static/fannst-banner.png)

# FSMTP Server Implemented Algorithms

## MIME Parsers
### Headers
Together these algorithms are capable of parsing most MIME messages 

1. Header key, value parser
1. Header value parser, for both keyed values an non keyed values

### Body

1. Multipart/alternative parser using boundary
1. HTML & Text parser
1. HTML to MarkDown

### Other parsers

1. Email address parser, name and address itself
1. Address splitter, to domain and username
