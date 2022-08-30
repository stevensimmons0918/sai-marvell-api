#! /usr/bin/perl
##! /scratch/dump6/tools/perl518/bin/perl -I /scratch/dump6/tools/perl518/lib
##! /tools/perl518/bin/perl -I /tools/perl518/lib

########################################
# this program takes on STDIN GCC preprocessed foobar.i file,
# from foobar.cpp or foobar.c
#
# and makes what is needed for a SWIG interface out of that
# returns that on stdout
#
# foobar.c (c suffix not cpp) is the argument
#
# right now, we just return inline method blocks
########################################

use lib 'SWIG';
use XPParse qw(
        preprocContents
        nestedDelimPat
);


# nested block pattern delimited by {}
$block = nestedDelimPat('\\{', '}');

$contents = preprocContents($ARGV[0]);
print $1 while
        $contents =~ /
                \b(inline\b
                [^\{]+  #part before the block
                $block
        )/gox;
