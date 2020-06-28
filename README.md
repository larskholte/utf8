utf8
====

UTF-8 is arguably the best encoding ever invented. It can represent all Unicode code points with a minimum of overhead, yet it is backwards-compatible with US-ASCII for characters up to U+007f (DEL). An encoded UTF-8 string never includes null bytes ('\0') unless the input sequence contains null characters, allowing all Unicode characters except null to be safely passed in C-strings.

UTF-8 is also the most popular encoding for use on the web. There is really no reason to use a different encoding in new software development unless backwards compatibility must be maintained with older encodings.

This is a small library to work with UTF-8 in C. A minimalistic approach was taken to development and the resulting library is fast and simple. See the header file for a description of the functions and the single data structure.
