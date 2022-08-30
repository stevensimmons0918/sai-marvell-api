########################################
# package parsing code
########################################
package XPParse;

use Exporter;
use vars '@EXPORT_OK';
our @ISA         = 'Exporter';
our @EXPORT_OK =  qw(
	preprocContents 
	nestedDelimPat
	cppStringSplitTopTokens
	printTokens
);


########################################
# this sub takes on STDIN:
# preprocessed C file by GCC (*.i)
# and as argument: prefix of one of the files comprising the above 
# (either the file being preprocessed, or one if its recursive includees)
#
# and returns the part of the first file that corresponds to the second file
#
# the intent is that the prefix contains at least the dot, and then that there is only one file
# to consider with that stem prefix
#
# the typical use of that is that if we don't know whether 
# we are looking for preprocessed contents
# of the .cpp or .c file, we just supply the .c file name
#
# we use the undocumented fact that a file preprocessed by GCC 
# will have comments of the type
# # line_number "file" ...
# and below that is a preprocessed fragment of that file, until the next comment
########################################
sub preprocContents {
	$one = $_[0];
	my $result;

	while (<STDIN>) {

		# $inOne means we are inside the file $one we care about
		if (m'^(?>
			\s*\#\s*	#start of comment
			[1-9]\d*\s+		#line number
			"([^"]+)"	#file name capture
		)'x) {
			$inOne = $1 =~ qr(^$one);
		}
		else {
			$result .= $_ if $inOne;
		}
	}

	return $result;
}


########################################
# this sub returns a regex pattern to match a string 
# between specified delimiters, possibly nested
# arguments are:
# left delimiter
# right delimiter
#
# arguments are specified as strings, which should interpret to what you want
# in a regular expression,
# so for example
# nestedDelimPat('\\(', '\\)')
# will produce a pattern that will match a string between nested paired ()
# for example 
#
# '(foobar(foobar()))'	will match
#
# the matched text is intended to be code, with ordinary quote "" rules:
# inside quotes:
#	the text is meant literally and your matching delimiters are ignored
#	quote is escaped with \
#	\ is escaped with \
#
# so for example
#
# '(foobar()"\"(")'	will match
# '(foobar()"\\"(")'		will not match
########################################
sub nestedDelimPat {
	$l = $_[0];
	$r = $_[1];

	# quote pattern
	$quotePat = qr("	#start quote

		# in the middle we match one escaped character at a time, in order:
		# first try to match \\
		# if not that, then \"
		# if not that, then anything (other than ")
		(?> (?: \\\\ | \\" | [^"] ) * )

	")x;	#end quote

	# pattern without the delimiters (except possibly in a quote)
	# as we go by the matched string, we match one suitable character at a time,
	# or else the whole quote
	$noDelimPat =  qr((?> (?: [^$l$r"] | $quotePat )*))x;

	# the nested delimiter pattern has to be evaluated partially at match-time
	# when the nested pair is needed to be matched, we then build up 
	# our pattern recursively
        use re q(eval);
	 $pat =  qr(
		$l		#left top delimiter
		$noDelimPat	# a part without delimiters

		# now part with a nested pair of delimiters - optional
		# plus another part without delimiters at the end - that can be also optional
		# since without delimiters, we would just need one above
		(?:
			(??{$pat})		#recursive step
			$noDelimPat
		)*
		$r		#right top delimiter
	)x;

	return $pat;
}


########################################
# split legal cpp code into array of tokens
# argument is a string of code - 
# 
# either preprocessed contents of one file -
# that means, a preprocessed file, restricted to contents of one file 
# (without other #include's)
# and without any comments starting with '#'
#
# or contents inside a block {} (also preprocessed already and coming from one file)
#
# return an array 
# each element corresponds to the code for:
#
# class, 
# function/method, 
# variable,
# or member visibility keyword
# 
# (elements are ordered as in the code)
# each element is in turn an array of strings:
# 
# if the element contains a block {}, like for example:
#
# class foobar {
# ...
# }
#
# then the element is "complext" and it is a reference to array
# of 3 strings:
# up to {
# inside block
# end }
#
# if the element does not contain a block then it is just one string
#
# element ends with the last meaningful character, and subsequent whitespace belongs to 
# the next element
########################################
sub cppStringSplitTopTokens {

	# return value (reference to)
	my @arr;

	#pattern for a block delimited by {}
	$block = nestedDelimPat('\\{', '}');

	#progressive match for successive tokens to correspond to outer array
	#capture the whole token in $1
	#other token without {} or ending ; , in $2
	#{} block for other token, in $3 

	while ($_[0] =~ /(\s*(?:	
	
		(?:(?: public | private | protected)\s*:) |	#visibility token

		([^\{";]+ )		#other token before {} or ;
		($block?)		# other token {}
		([\{";]?)

	))/gox) {
	
		# if we have a block then separate the token into array of 3
		if ($3) {
			push (@arr, [$2.'{', substr($3, 1, -1), '}'.$4]);
		}

		# if not, the whole token in 1 element of inner array
		else {
			push(@arr, $1);
		}

	 };

	 return \@arr;
 }


 ########################################
 # $_[0] is an array of "tokens" - each is either a string or a reference to a recursive array
 # everything eventually ends in a string
 # this sub prints the contents of tokens to file handle $_[1]
 #
 # typically tokens are code fragments, 
 # a string means that it is not further parsed, 
 # recursive array means it has been further parsed
 ########################################
 sub printTokens {
	my $fh = $_[1];
	for $token (@{$_[0]}) {
		if (ref $token) {
			bless $token;
			printTokens $token $fh;
		}
		else {
			print $fh $token;
		}
	}
 }


########################################
# return the name of class, method or variable for a cpp code fragment $_[0]
#
# we simply take the last word before a delimiter such as { , ( , or ;
########################################
sub cppTokenName {
	$_[0] =~ '\b(\w+)\s*(?:{|\(|;)';
	return $1;
}


1;
