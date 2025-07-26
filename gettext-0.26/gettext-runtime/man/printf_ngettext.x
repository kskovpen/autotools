.\" Copyright (c) Free Software Foundation, Inc.
.\"
.\" This is free documentation; you can redistribute it and/or
.\" modify it under the terms of the GNU General Public License as
.\" published by the Free Software Foundation; either version 2 of
.\" the License, or (at your option) any later version.
.\"
.\" Written by Bruno Haible <bruno@clisp.org>.
.\"
[NAME]
printf_ngettext \- translate format string and apply it
[DESCRIPTION]
.\" Add any additional description here
The \fBprintf_ngettext\fP program translates a format string into the user's
language, by looking up the translation in a message catalog and then
choosing the appropriate plural form, which depends on the number \fICOUNT\fP
and the language of the message catalog where the translation was found, and
applies the translated format string to the specified arguments.
