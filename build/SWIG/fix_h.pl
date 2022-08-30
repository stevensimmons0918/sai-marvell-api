#! /usr/bin/perl5.18.2
##! /scratch/dump6/tools/perl518/bin/perl -I /scratch/dump6/tools/perl518/lib
##! /tools/perl518/bin/perl -I /tools/perl518/lib

########################################
# this program fixes GCC-preprocessed  header file $ARGV[0] 
# so it is understood by SWIG and returns the new file, name with .i appended
#
# currently we do the following:
#  workaround nested classes - 
#	explained as second workaround in Nested Classes section of SWIG manual
#
# 
# current limitations:
#
#	nested class SubClass is assumed to occur only inside its class Class
#  so that, there is no Class::SubClass outside Class
# 
# each class declaration occurs completely in one file
# 
# ---------------------------------
#
# the input file is GCC preprocessed .h code, with option -E, so it has 
# specific preprocessor comments as understood by subroutine preprocContents
# in module XPParse
# 
# we use these comments to only consider the parts of code, that correspond to preprocessed $ARGV[1] file 
#
# so if $ARGV[0] is some subdir/foobar.i , then
# $ARGV[1] can be:
# subdir/foobar.h , or 
# one of the files #included in that
#
# from the resulting code we remove nested subclasses, and then,
# any methods in the outer class, that use the nested subclasses as args or return
########################################

use re 'eval';
use lib 'SWIG';
use XPParse qw(
	preprocContents
	cppStringSplitTopTokens
	printTokens
);


########################################
# return the array reference [name, block inside {}]
# for a class parsed as in XPParse cppStringSplitTopTokens function
# or [false, false] if not a class
########################################
sub classNameBlock {
	$arg = $_[0];
	if (ref $arg && $arg->[0] =~ '\bclass\s+(\w+)') {
		return [$1, $arg->[1]];
	}
	else {
		return '';
	}
}


########################################
# for cpp tokens parsed as in XPParse cppStringSplitTopTokens
# remove these tokens that have unwanted symbols in declarations
#
# $_[0] is a reference to array of tokens
# $_[1] is an array of strings, each of which is a symbol we want to avoid
# 
# if a token has one of these symbols, before {} if the token is parsed that way:
# into  the part before { and the block, 
#
# then we remove that token (substitute empty string for it)
########################################
sub topTokensRmUnwantedCppDeclareRef {
	$arg = $_[0];

	# regex pattern we are looking for
	$pat = '\b(?:'.join('|', @{$_[1]}).')\b';
	$pat = qr($pat);

	for $i (0 .. $#{$arg}) {
		$element = $arg->[$i];
		$declaration = ref $element ? $element->[0] : $element;
		$arg->[$i] = '' if $declaration =~ $pat;
	}
}


########################################
# main program
########################################

$in = $ARGV[0];

# we want to restrict the preprocessed file to this header file
# preprocessed contents (yes we are assuming whole classes declarations are in one file)
($mainH = $in) =~ s?\.i.*?\.h?;
$fileContents = preprocContents($in, $mainH);

# split into top-level tokens as explained in XParse module 
@tokens = @{cppStringSplitTopTokens($fileContents)};
for  $i (0 .. $#tokens) {

	# if we have a class with a block, then we recursively split the block
	# looking for sub-classes
	($className, $classBlock) = @{classNameBlock($tokens[$i])};
	if ($classBlock) {
		my @tokens1 = @{cppStringSplitTopTokens($classBlock)};

		for $j (0 .. $#tokens1) {
			$token1 = $tokens1[$j];

			# if the second-order token has a class block (nested class)
			# then implement "global Inner class" workaround from the SWIG manual
			# section on Nested Classes:
			# move the whole subclass token from inside outer class to the top scope, and
			# typedef Inner as nested

			($innerClassName, $junk) = @{classNameBlock($token1)};
			if ($innerClassName) {
				$tokens1[$j] = '';
				printTokens [$token1];
				print <<END;

%nestedworkaround $className::$innerClassName;

%{
// SWIG thinks that Inner is a global class, so we need to trick the C++
// compiler into understanding this so called global type.
typedef $className::$innerClassName $innerClassName;
%}
END

			}
		}

		# remove unwanted stuff (in the future we will probably replace it)
		topTokensRmUnwantedCppDeclareRef(\@tokens1, ["operator"]);

		# replace the top-level token class block, with the recursively parsed blcck
		# with nested classes removed
		$tokens[$i][1] = \@tokens1;
	}
}

# not sure how, but there are operators declared outside class scope, nuke them
topTokensRmUnwantedCppDeclareRef(\@tokens, ["operator"]);
printTokens(\@tokens);
