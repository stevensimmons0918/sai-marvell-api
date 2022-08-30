#! /usr/bin/perl5.18.2
##! /scratch/dump6/tools/perl518/bin/perl -I /scratch/dump6/tools/perl518/lib
##! /tools/perl518/bin/perl -I /tools/perl518/lib

########################################
# this program takes GCC preprocessed .i files, from .cpp and .c files
# and makes what is needed for a SWIG interface out of that
# returns that on stdout
#
# right now, we just return inline method blocks
########################################

use lib 'SWIG';
use XPParse qw(
	preproc_contents
	nested_delim_pat
);


# nested block pattern delimited by {}
$block = nested_delim_pat('\\{', '}');

for (@ARGV) {
	$contents = preproc_contents($_, substr($_, 0, -2).'.c');
	print $1 while  
		$contents =~ /
			\b(inline\b
			[^\{]+	#part before the block
			$block
		)/gox;
}
