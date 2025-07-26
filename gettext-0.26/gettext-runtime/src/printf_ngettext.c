/* Formatted output with a plural form of a localized format string.
   Copyright (C) 2025 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2025.  */

/* This program is a combination of the 'ngettext' program with the 'printf'
   program.  It takes the (English) singular and plural form of a format string,
   a cardinal number, and arguments. It finds the translation of the format
   string (for the current locale, according to the environment variables
   TEXTDOMAIN and TEXTDOMAINDIR), by looking it up in a message catalog and
   then choosing the appropriate plural form, which depends on the number and
   the language of the message catalog where the translation was found, and
   applies that translated format string to the arguments.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <error.h>
#include "options.h"
#include "printf-command.h"
#include "noreturn.h"
#include "closeout.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);

int
main (int argc, char *argv[])
{
  /* Default values for command line options.  */
  bool do_help = false;
  bool do_version = false;
  const char *domain = getenv ("TEXTDOMAIN");
  const char *domaindir = getenv ("TEXTDOMAINDIR");
  const char *context = NULL;

  /* Set program name for message texts.  */
  set_program_name (argv[0]);

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  bindtextdomain ("gnulib", relocate (GNULIB_LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Parse command line options.  */
  {
    BEGIN_ALLOW_OMITTING_FIELD_INITIALIZERS
    static const struct program_option options[] =
    {
      { "context", 'c', required_argument },
      { "help",    'h', no_argument       },
      { "version", 'V', no_argument       },
    };
    END_ALLOW_OMITTING_FIELD_INITIALIZERS
    start_options (argc, argv, options, NON_OPTION_TERMINATES_OPTIONS, 0);
    int optchar;
    while ((optchar = get_next_option ()) != -1)
      switch (optchar)
        {
        case '\0':          /* Long option with key == 0.  */
          break;
        case 'c':
          context = optarg;
          break;
        case 'h':
          do_help = true;
          break;
        case 'V':
          do_version = true;
          break;
        default:
          usage (EXIT_FAILURE);
        }
  }

  /* Version information is requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", last_component (program_name),
              PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
License GPLv3+: GNU GPL version 3 or later <%s>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
"),
              "2025", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Bruno Haible"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* The format string is the first non-option argument.  */
  if (!(argc - optind >= 3))
    {
      error (EXIT_SUCCESS, 0, _("missing arguments"));
      usage (EXIT_FAILURE);
    }
  const char *format = argv[optind++];
  const char *format_plural = argv[optind++];
  const char *count = argv[optind++];

  unsigned long n;
  {
    char *endp;
    unsigned long tmp_val;

    if (isdigit ((unsigned char) count[0])
        && (errno = 0,
            tmp_val = strtoul (count, &endp, 10),
            errno == 0 && endp[0] == '\0'))
      n = tmp_val;
    else
      /* When COUNT is not valid, use plural.  */
      n = 99;
  }

  argc -= optind;
  argv += optind;

  /* The number of arguments consumed in each processing round is determined
     by the FORMAT and FORMAT-PLURAL arguments.  This is necessary to avoid
     havoc if the translated format string happens to consume a different
     number of arguments.  */
  size_t args_each_round;
  {
    size_t args_consumed_1 = printf_consumed_arguments (format);
    size_t args_consumed_2 = printf_consumed_arguments (format_plural);
    args_each_round =
      (args_consumed_1 >= args_consumed_2 ? args_consumed_1 : args_consumed_2);
  }

  if (domain != NULL && domain[0] != '\0')
    {
      /* Bind domain to appropriate directory.  */
      if (domaindir != NULL && domaindir[0] != '\0')
        bindtextdomain (domain, domaindir);

      /* Look up the localized format string.  */
      format = (context != NULL
                ? dnpgettext_expr (domain, context, format, format_plural, n)
                : dngettext (domain, format, format_plural, n));
    }
  else
    /* Use English plural form handling.  */
    format = (n == 1 ? format : format_plural);

  /* Execute a 'printf' command.  */
  printf_command (format, args_each_round, argc, argv);

  exit (EXIT_SUCCESS);
}


/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try '%s --help' for more information.\n"),
             program_name);
  else
    {
      /* xgettext: no-wrap */
      printf (_("\
Usage: %s [OPTION] FORMAT FORMAT-PLURAL COUNT [ARGUMENT]...\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Produces formatted output, applying the native language translation of FORMAT\n\
and FORMAT-PLURAL, depending on COUNT, to the ARGUMENTs.\n"));
      printf ("\n");
      printf (_("\
Options and arguments:\n"));
      printf (_("\
  -c, --context=CONTEXT     specify context for FORMAT\n"));
      printf (_("\
  FORMAT                    English singular form of format string\n"));
      printf (_("\
  FORMAT-PLURAL             English plural form of format string\n"));
      printf (_("\
  COUNT                     choose singular/plural form based on this value\n"));
      printf (_("\
  ARGUMENT                  string or numeric argument\n"));
      printf ("\n");
      printf (_("\
Informative output:\n"));
      printf (_("\
  -h, --help                display this help and exit\n"));
      printf (_("\
  -V, --version             display version information and exit\n"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Each format string consists of\n\
  - plain text,\n\
  - directives, that start with '%c',\n\
  - escape sequences, that start with a backslash.\n"),
              '%');
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
A directive that consumes an argument\n\
  - starts with '%s' or '%s' where %s is a positive integer,\n\
  - is optionally followed by any of the characters '%c', '%c', '%c', '%c', '%c',\n\
    each of which acts as a flag,\n\
  - is optionally followed by a width specification (a nonnegative integer),\n\
  - is optionally followed by '%c' and a precision specification (an optional\n\
    nonnegative integer),\n\
  - is finished by a specifier\n\
      - '%c', that prints a character,\n\
      - '%c', that prints a string,\n\
      - '%c', '%c', that print an integer,\n\
      - '%c', '%c', '%c', '%c', that print an unsigned (nonnegative) integer,\n\
      - '%c', '%c', that print a floating-point number in scientific notation,\n\
      - '%c', '%c', that print a floating-point number without an exponent,\n\
      - '%c', '%c', that print a floating-point number in general notation,\n\
      - '%c', '%c', that print a floating-point number in hexadecimal notation.\n\
Additionally there is the directive '%s', that prints a single '%c'.\n"),
              "%", "%m$", "m",
              '#', '0', '-', ' ', '+',
              '.',
              'c',
              's',
              'i', 'd',
              'u', 'o', 'x', 'X',
              'e', 'E', 'f', 'F', 'g', 'G', 'a', 'A',
              "%%", '%');
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
If a directive specifies the argument by its number ('%s' notation),\n\
all directives that consume an argument must do so.\n"),
              "%m$");
      printf ("\n");
      /* TRANSLATORS: Most of the placeholders expand to 2 characters.
         The last placeholder expands to 4 characters.  */
      printf (_("\
The escape sequences are:\n\
\n\
  %s      backslash\n\
  %s      alert (BEL)\n\
  %s      backspace (BS)\n\
  %s      form feed (FF)\n\
  %s      new line (LF)\n\
  %s      carriage return (CR)\n\
  %s      horizontal tab (HT)\n\
  %s      vertical tab (VT)\n\
  %s    octal number with 1 to 3 octal digits\n"),
              "\\\\", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v",
              "\\nnn");
      printf ("\n");
      printf (_("\
Environment variables:\n"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
The translation of the format string is looked up in the translation domain\n\
given by the environment variable %s.\n"),
              "TEXTDOMAIN");
      /* xgettext: no-wrap */
      printf (_("\
It is looked up in the catalogs directory given by the environment variable\n\
%s or, if not present, in the default catalogs directory.\n\
This binary is configured to use the default catalogs directory:\n\
%s\n"),
              "TEXTDOMAINDIR",
              getenv ("IN_HELP2MAN") == NULL ? relocate (LOCALEDIR) : "@localedir@");
      printf ("\n");
      /* TRANSLATORS: The first placeholder is the web address of the Savannah
         project of this package.  The second placeholder is the bug-reporting
         email address for this package.  Please add _another line_ saying
         "Report translation bugs to <...>\n" with the address for translation
         bugs (typically your translation team's web or email address).  */
      printf (_("\
Report bugs in the bug tracker at <%s>\n\
or by email to <%s>.\n"),
              "https://savannah.gnu.org/projects/gettext",
              "bug-gettext@gnu.org");
    }

  exit (status);
}
