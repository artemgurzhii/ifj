//
// ifj17.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "ifj17.h"
#include "codegen.h"
#include "errors.h"
#include "lexer.h"
#include "linenoise.h"
#include "parser.h"
#include "prettyprint.h"
#include "utils.h"
#include "vm.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// --ast

static int ast = 0;

// --tokens

static int tokens = 0;

/*
 * Output usage information.
 */

void usage() {
  fprintf(stderr, "\n  Usage: ifj17 [options] [file]"
                  "\n"
                  "\n  Options:"
                  "\n"
                  "\n    -A, --ast       output ast to stdout"
                  "\n    -T, --tokens    output tokens to stdout"
                  "\n    -h, --help      output help information"
                  "\n    -V, --version   output ifj17 version"
                  "\n"
                  "\n  Examples:"
                  "\n"
                  "\n    $ ifj17 < some.ifj17"
                  "\n    $ ifj17 some.ifj17"
                  "\n    $ ifj17 some"
                  "\n    $ ifj17"
                  "\n"
                  "\n");
  exit(1);
}

/*
 * Output ifj17 version.
 */

void version() {
  printf("%s\n", IFJ17_VERSION);
  exit(0);
}

/*
 * Line-noise REPL.
 */

void repl() {
  ifj17_set_prettyprint_func(printf);
  char *line;
  while ((line = linenoise("ifj17> ")))
  {
    if ('\0' != line[0])
    {
      // parse the input
      ifj17_lexer_t lex;
      ifj17_lexer_init(&lex, line, "stdin");
      ifj17_parser_t parser;
      ifj17_parser_init(&parser, &lex);
      ifj17_block_node_t *root;

      // oh noes!
      if (!(root = ifj17_parse(&parser)))
      {
        ifj17_report_error(&parser);
        exit(1);
      }

      // print
      ifj17_prettyprint((ifj17_node_t *)root);
      linenoiseHistoryAdd(line);
    }
    free(line);
  }
  exit(0);
}

/*
 * Parse arguments.
 */

const char **parse_args(int *argc, const char **argv) {
  const char *arg, **args = argv;

  for (int i = 0, len = *argc; i < len; ++i)
  {
    arg = args[i];
    if (!strcmp("-h", arg) || !strcmp("--help", arg))
      usage();
    else if (!strcmp("-V", arg) || !strcmp("--version", arg))
      version();
    else if (!strcmp("-A", arg) || !strcmp("--ast", arg))
    {
      ast = 1;
      --*argc;
      ++argv;
    } else if (!strcmp("-T", arg) || !strcmp("--tokens", arg))
    {
      tokens = 1;
      --*argc;
      ++argv;
    } else if ('-' == arg[0])
    {
      fprintf(stderr, "unknown flag %s\n", arg);
      exit(1);
    }
  }

  return argv;
}

/*
 * Evaluate `source` with the given
 * `path` name and return status.
 */

int eval(char *source, const char *path) {
  // parse the input
  ifj17_lexer_t lex;
  ifj17_lexer_init(&lex, source, path);
  ifj17_parser_t parser;
  ifj17_parser_init(&parser, &lex);
  ifj17_block_node_t *root;

  // --tokens
  if (tokens)
  {
    while (ifj17_scan(&lex))
    {
      printf("  \e[90m%d : \e[m", lex.lineno);
      ifj17_token_inspect(&lex.tok);
    }
    return 0;
  }

  // oh noes!
  if (!(root = ifj17_parse(&parser)))
  {
    ifj17_report_error(&parser);
    return 1;
  }

  // --ast
  ifj17_set_prettyprint_func(printf);
  ifj17_prettyprint((ifj17_node_t *)root);

  // evaluate
  ifj17_vm_t *vm = ifj17_gen((ifj17_node_t *)root);
  ifj17_object_t *obj = ifj17_eval(vm);
  ifj17_object_inspect(obj);
  ifj17_object_free(obj);
  ifj17_vm_free(vm);

  return 0;
}

/*
 * Parse arguments and scan from stdin (for now).
 */

int main(int argc, const char **argv) {
  int tried_ext = 0;
  const char *path, *orig;
  char *source;

  // parse arguments
  argv = parse_args(&argc, argv);

  // eval stdin
  if (argc == 1 && isatty(0) == false) {
    source = read_until_eof(stdin);
    return eval(source, "stdin");
  }

  // REPL
  if (argc == 1)
    repl();

  // eval file
  orig = path = argv[1];
read:
  if (!(source = file_read(path)))
  {
    // try with .ifj17 extension
    if (!tried_ext)
    {
      tried_ext = 1;
      char buf[256];
      snprintf(buf, 256, "%s.ifj17", path);
      path = buf;
      goto read;
    }
    fprintf(stderr, "error reading %s:\n\n  %s\n\n", orig, strerror(errno));
    exit(1);
  }

  return eval(source, path);
}
