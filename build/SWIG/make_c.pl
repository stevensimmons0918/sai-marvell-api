#! /usr/bin/perl
##! /scratch/dump6/tools/perl518/bin/perl -I /scratch/dump6/tools/perl518/lib
##! /tools/perl518/bin/perl -I /tools/perl518/lib

########################################
# this program translates a g++-preprocessed  C++ source file on STDIN
# so it is understood by SWIG, into the new source file on STDOUT
#
# if there is an additional argument $ARGV[0], then in that file are listed
# (one on a line)
# "front-end" files that are included in our source, that we want to translate
#
# that is, the translation is limited to the preprocessed parts that are coming from
# these front-end files
#
#
# currently we do the following:
#
#  replace any array declaration
#       type name[size];
# with
#       _wrapped_arr_type_size name;
#
# size has to be a constant expression after preprocessing, which can be evaluated
# by Perl
#
# if type is
# (un)signed something
# these two words are joined with '_' ,
# so that
# _wrapped_arr_type_size
# is still one word
#
#
# this works recursively as well:
# if you have
# typedef type0 type1[size];
# typedef type1 type2;
#
# then
# type2 name;
# is still replaced with
# _wrapped_arr_type0_size name;
#
# another program will scan this new header file and define
# _wrapped_arr_type_size elsewhere to be a struct whose only field is
#       type name[size]
#
########################################

use Cwd 'fast_abs_path';

# this variable is false only when we know for sure we are not
# in the preprocessed front-end
$frontEnd = 1;

# enter front end files as keys in %frontEnd
if (@ARGV) {
        open(FH, "$ARGV[0]") or die "could not open $ARGV[0]";
        while ($_ = <FH>) {
                chomp;
                $frontEnd{fast_abs_path $_} = undef;
        }
}

my $matchFound = 0;
# read STDIN line by line
# $contents accumulates read stuff that we have not processed yet
while (1) {

        $_ = <STDIN>;
        $contents .= $_;

        # match for preprocessing comment that signifies the start of section coming from
        # a particular included file

        # if we match, that means the previous such section has ended and we can process it
        # also if we have no more input, we are at end and we can process what we have last
        # accumulated
        if (m'^
                        \s*\#\s*        #start of comment
                        [1-9]\d*\s+             #line number
                        "([^"]*)"               # the file matched as $1
                'x || !$_) {
                #there could be some spurious phrases matched, such as <command-line>
                # not a real file
                next unless -e $1;

                #there could be some spurious phrases matched, such as <command-line>
                $file = $1;

                #there could be some spurious phrases matched, such as <command-line>
                #hash $arrays{$newType}
                # has keys that are known to be types recursively typedefed to arrays
                # the value has the recursive replacement string
                #
                # in the example above, we would have
                # $arrays{$type1} equal "_wrapped_arr_type0_size"
                # $arrays{$type2} also equal the same

                # we first update %arrays over the entire $contents
                # then we will do global replacement over the same
                #
                # this is convenient
                # that means we go back and rescan stdin, and typedef has forward scope only
                # but: we cannot go wrong - if new type introduced by a typedef were used
                # previously, that would be a C syntax error

                while ($contents =~
                        /\btypedef\s+

                        # the existing type matched as $1, possibly not one but two words
                        (\w+(?:\s+\w+)?)\s+

                        (\w+)\s*        # new type matched as $2
                        (\[([^\]]+)\])?         # optional [size] matched as $3
                        \s*;/gx) {

                        # if we have size, that is the original array and start of possible recursion
                        if ($3) {
                                $newType = $2;
                                $num = eval($4);

                                # if type $1 is two words, join them
                                ($type = $1) =~ s/\s+/_/;
                                $arrays{$newType} = '_wrapped_arr_'.$type."_$num";
                        }
                        else {
                                # if we do not have size, then recursively propagate existing replacement string only
                                $arrays{$2} = $arrays{$1} if exists $arrays{$1}
                        }
                }

                # now global replacement substitution, but only if that were front end, or we don't care
                # replace with a programmed string

                $contents =~ s/(
                        #$1 whole matched substring needed if we do not want to substitute after all

                        #$2 possible typedef - looks like a declaration but should not be replaced
                        (\btypedef\s+)?

                        #$3 and optional $4 type, but could also be C keyword 'return' or 'sizeof'
                        (\w+)(?:\s+(\w+))?\s+

                        (\w+)\s*                                                #$5 name of variable
                        (\[(\d+)\])?                            #$7 possible size of array
                        \s*;)/

                        # typedef or a keyword, do not replace
                        $2 || $3 eq 'return' || $3 eq 'sizeof' ? $1 :

                        # else, if we have a size then replace for sure
                        # if two words in type, join them
                        $6 ? "_wrapped_arr_$3".($4 ? "_$4" : '').'_'.eval($7)." $5;" :

                        # else, if type is recursively an array, replace with the hashed string
                        exists $arrays{$3} ? "$arrays{$3} $5;" :

                        # else again do not replace
                        $1/egx

                if $frontEnd;

                # update front end information for the upcoming section of stdin
                $frontEnd = exists $frontEnd{fast_abs_path $file} if @ARGV;

                print $contents;
                if($matchFound){
                        if($contents =~ /SWIG\/xpApp_wrap.cxx/ or $contents =~ /SWIG\/xpsApp_wrap.cxx/ or $contents =~ /SWIG\/sdk_wrap.cxx/ or $contents =~ /SWIG\/sdkTest_wrap.cxx/ or $contents =~ /SWIG\/openXps_wrap.cxx/ or $contents =~ /SWIG\/saiShell_wrap.cxx/ or $contents =~ /SWIG\/ksdk_wrap.cxx/ or $contents =~ /SWIG\/ksaiShell_wrap.cxx/) {
                                print "\n}\n";
                                $matchFound =0;
                        }
                }
                else{
                        if($contents =~ /1\s\".\/..\/pipeline-profiles\/xpDefault\/demo\//) {
                                $matchFound = 1;
                                print "\nextern \"C\" {\n";
                        }
                }
                $contents = '';
                last unless $_;

        }
}
