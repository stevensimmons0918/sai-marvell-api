#! /usr/bin/perl
##! /scratch/dump6/tools/perl518/bin/perl -I /scratch/dump6/tools/perl518/lib
##! /tools/perl518/bin/perl -I /tools/perl518/lib

########################################
# this program constructs SWIG/foobar.gen.*, its first argument
# scans all its (further) arguments which are generated SWIG .i interface
# and outputs common interface code that needs to be put before
# all of them
#
# for each _wrapped_arr_type_n found
# it outputs this as a struct:
#
# typedef struct {
#               type arr[n];
#               operator [](int n) { return arr[n];}
# } _wrapped_arr_type_n;
#
# and this only for SWIG interface .gen.i output:
#
# %extend _wrapped_arr_type_n {
#        type __get/setitem__(int index) {
#               return self->arr[index];
#       }
# };
########################################

$output = $ARGV[0];
shift;

open(FH, ">$output") or die "could not open $output";

$swig_output = $output =~ /\.gen\.i$/;

if (@ARGV) {
        while (<>) {
                while (/\b(_wrapped_arr_(.*)_(\d+))\b/g) {

                        # have a hash of seen wrapped arrays to be declared
                        # so we don't declare them twice
                        if (! exists $seen{$1}) {
                                $wrap = $1;
                                $size = $3;
                                $seen{$wrap} = undef;

                                # type may be two words joined, unjoin them
                                $type = $2;
                                $type=~ s/^((?:un)?signed)_/"$1 "/e;
                                print FH "typedef struct { $type arr[$size]; $type operator [](int n) { return arr[n];} } $wrap;\n";
                                if ($swig_output) {
                                        print FH <<END;
        %extend $wrap {
                $type __getitem__(int i) {
                        return self->arr[i];
                }
                void __setitem__(int i, $type val) {
                        self->arr[i] = val;
                }
        }
END
                                }
                        }
                }
        }
}
