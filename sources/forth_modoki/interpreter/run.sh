#!/bin/sh

if [ "$DEBUG" ]; then
    DEBUGOPT='-g -O0 -fsanitize=address'
fi

case "$1" in
    main)
        gcc $DEBUGOPT -o fm cl_getc.c token.c util.c parser.c stack.c dict.c eval.c && ./fm
        ;;
    eval-test)
        gcc $DEBUGOPT -DEVAL_TEST -o eval-test cl_getc.c token.c util.c parser.c stack.c dict.c eval.c && ./eval-test
        ;;
    dict-test)
        gcc $DEBUGOPT -DDICT_TEST -o dict-test element.c util.c stack.c dict.c && ./dict-test
        ;;
    stack-test)
        gcc $DEBUGOPT -DSTACK_TEST -o stack-test element.c util.c stack.c && ./stack-test
        ;;
    parser-test)
        gcc $DEBUGOPT -DPARSER_TEST -o parser-test cl_getc.c token.c util.c parser.c && ./parser-test
        ;;
    *)
        echo "Usage: $0 PROG_NAME"
        echo ""
        echo "PROG_NAME:"
        echo "    main                Compile and run the forth_modoki interpreter."
        echo "    stack-test          Compile and run tests for stacks."
        echo "    dict-test           Compile and run tests for dictionaries."
        echo "    parser-test         Compile and run tests for parser."
        echo "    eval-test           Compile and run tests for eval."
        ;;
esac
