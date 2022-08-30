////////////////////////////////////////
//common SWIG interface
////////////////////////////////////////

%{
#include <stdio.h>
%}

////////////////////////////////////////
// STATIC_CAST_POINTERS(fromType, toType)
// 
// translates from C++:
// static_case<toType*>(fromType*)
//
// to Python:
// toTypeObject = to_toType(fromTypeObject)
////////////////////////////////////////

%define STATIC_CAST_POINTERS(fromType, toType)
	%inline %{
		toType* to_##toType(fromType* from) {
			return static_cast<toType*>(from);
		}
	%}
%enddef


%pythoncode %{
	false = 0
	true = 1
%}
////////////////////////////////////////
// ARR_ARG(type)
//
// translates from C++:
// 
// method(type arg[anySize])
// or
// typedef type TYPE[anySize]
// method(TYPE arg)
//
// to Python:
// method([n1, n2, ..., nanySize])
//
// 
// method can be also a function
// method can have other arguments
// any integer signed or unsigned type, including types typedef'd to integer
// can be specified
// but all values of the integer type must be representable by Python integer
// 
// we check that the argument given on Python command line
// is a sequence of numbers
//
// if size is specified, we check the size of the sequence is correct
//
// but we do not check the size of the integers in the sequence - they must 
// be representable with type 
////////////////////////////////////////
%define ARR_ARG(type)

%typemap(in) type [ANY] (  type temp[$1_dim0]) {

  int i;

  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }

  if (PySequence_Length($input) != $1_dim0) {
		char str[64];
		sprintf(str, "Size mismatch. Expected %d elements", $1_dim0);
		PyErr_SetString(PyExc_ValueError, str);
		return NULL;
  }

  for (i = 0; i < $1_dim0; i++) {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyNumber_Check(o)) {
      temp[i] = PyInt_AsLong(o);
    } else {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");      
      return NULL;
    }
  }

  $1 = temp;
}

%typemap(argout) type [ANY] {
	int i;

	// sanity checked on the "in" direction, no need to repeat

	for (i = 0; i < $1_dim0; i++) {
		PySequence_SetItem($input, i, PyInt_FromLong($1[i]));
	}
}

%enddef



%define ARR_ARG_CLASS(type)
%typemap(memberout) type [ANY]{

        int i;
        PyObject *o;
        PyObject *elementObj;

        $result = PyList_New($1_dim0);
        for (i = 0; i < $1_dim0; i++) {
	        elementObj = SWIG_NewPointerObj(&$1[i], $descriptor(type),0 );     
		PyList_SetItem($result, i,  elementObj );
		}  

}
%typemap(memberin) type [ANY]{
// Write access is not added
}
%enddef



////////////////////////////////////////
// ARR2_ARG(type)
//
// is like the above, but for 2-dimensional arrays of any size and
// type type
////////////////////////////////////////
%define ARR2_ARG(type)

%typemap(in) type [ANY][ANY] (  type temp[$1_dim0][$1_dim1]) {

  int i, j;
	char str[64];

  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }

  if (PySequence_Length($input) != $1_dim0) {
		
		sprintf(str, "Size mismatch. Expected %d elements in the outer sequence", $1_dim0);
		PyErr_SetString(PyExc_ValueError, str);
		return NULL;
  }

  for (i = 0; i < $1_dim0; i++) {
    PyObject *o = PySequence_GetItem($input,i);

	if (!PySequence_Check(o)) {
		sprintf(str, "As %d element of the outer sequence: expected a sequence", i);
		PyErr_SetString(PyExc_ValueError, str);
		return NULL;
	}

	if (PySequence_Length(o) != $1_dim1) {
		sprintf(str, "%d inner sequence: size mismatch. Expected %d elements", i, $1_dim1);
		PyErr_SetString(PyExc_ValueError, str);
		return NULL;
  }

	for (j = 0; j < $1_dim1; j++) {
		PyObject *oo = PySequence_GetItem(o,j);

		if (PyNumber_Check(oo)) {
			temp[i][j] = PyInt_AsLong(oo);
		} else {
			sprintf(str, "%d inner sequence elements must be numbers", i);      
			PyErr_SetString(PyExc_ValueError, str);
			return NULL;
		}
	}
  }
  $1 = temp;
}

%typemap(argout) type [ANY][ANY] {

	int i, j;
	PyObject *o;

	// sanity checked on the "in" direction, no need to repeat

	for (i = 0; i < $1_dim0; i++) {
		o = PySequence_GetItem($input, i);
		for (j = 0; j < $1_dim1; j++) {
			PySequence_SetItem(o, j, PyInt_FromLong($1[i][j]));
		}
	}
  }
  
  
%typemap(varout) type [ANY][ANY] {

        int i, j;
        PyObject *o;
        
        $result = PyList_New($1_dim0);
        for (i = 0; i < $1_dim0; i++) {
                o = PyList_New($1_dim1);
                PyList_Insert($result, i, o);
                for (j = 0; j < $1_dim1; j++) {
                  
                    PyList_SetItem(o, j, PyInt_FromLong($1[i][j]));
                }
		}  
}
%enddef


%define ARR2_ARG_CLASS(type)

%typemap(varout) type [ANY][ANY]{

        int i, j;
        PyObject *o;
        PyObject *elementObj;

        $result = PyList_New($1_dim0);
        for (i = 0; i < $1_dim0; i++) {
                o = PyList_New($1_dim1);
                PyList_Insert($result, i, o);
                for (j = 0; j < $1_dim1; j++) {
				 
					elementObj = SWIG_NewPointerObj(&$1[i][j], $descriptor(type *),0 );     
					PyList_SetItem(o, j,  elementObj );
				}
		}  
}

%typemap(varin) type [ANY][ANY]{
// Write access is not added
}
%enddef


////////////////////////////////////////
// ARR3_ARG(type)
//
// is like the above, but for 3-dimensional arrays of any size and
// type type
////////////////////////////////////////
%define ARR3_ARG(type)

/*
%typemap(varout) type [ANY][ANY][ANY] {

        int i, j, k;
        //PyObject *o;
        PyObject *o = 0;
        pyobj = SWIG_NewPointerObj(SWIG_as_voidptr(xp80HashCfg), SWIGTYPE_p_a_1__xpHashTableContext,  0 );
        PyObject *oo = 0;
        pyobjk = SWIG_NewPointerObj(SWIG_as_voidptr(xp80HashCfg), SWIGTYPE_p_a_1__xpHashTableContext,  0 );
        //PyObject *oo;

        $result = PyList_New($1_dim0);
        for (i = 0; i < $1_dim0; i++) {
                pyobj = PyList_New($1_dim1);
                PyList_Insert($result, i, pyobj);
                for (j = 0; j < $1_dim1; j++) {
                        oo = PyList_New($1_dim2);
                        PyList_Insert(o, j, oo);                 
                        for (k = 0; k < $1_dim2; k++) {
                                //PyList_SetItem(oo, k, PyInt_FromLong($1[i][j][k]));
                                PyList_SetItem(&oo, k,$1[i][j][k]);
                        }
                }  
        }
}
*/


%typemap(varout) type [ANY][ANY][ANY] {

        int i, j, k;
        PyObject *o;
        PyObject *oo;
        PyObject *elementObj;

        $result = PyList_New($1_dim0);
        for (i = 0; i < $1_dim0; i++) {
                o = PyList_New($1_dim1);
                PyList_Insert($result, i, o);
                for (j = 0; j < $1_dim1; j++) {
                        oo = PyList_New($1_dim2);
                        PyList_Insert(o, j, oo);                 
                        for (k = 0; k < $1_dim2; k++) {
                        //elementObj = SWIG_NewPointerObj(&$1[i][j][k], SWIGTYPE_p_xpHashTableContext,0 );     
                        elementObj = SWIG_NewPointerObj(&$1[i][j][k], $descriptor(type *),0 );     
                        PyList_SetItem(oo, k,  elementObj );
                        }
                }  
        }
}





%typemap(varin) type [ANY][ANY][ANY]  {
/*
  int i, j,k;
  char str[64];
  //int $1[1][1][1]; //PyObject *($input);
  //int temp[1][1][1];// = PyObject *($input);
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }

  if (PySequence_Length($input) != $1_dim0) {
                
                sprintf(str, "Size mismatch. Expected %d elements in the outer sequence", $1_dim0);
                PyErr_SetString(PyExc_ValueError, str);
                return NULL;
  }

  for (i = 0; i < $1_dim0; i++) {
    PyObject *o = PySequence_GetItem($input,i);

                if (!PySequence_Check(o)) {
                        sprintf(str, "As %d element of the outer sequence: expected a sequence", i);
                        PyErr_SetString(PyExc_ValueError, str);
                        return NULL;
                }

                if (PySequence_Length(o) != $1_dim1) {
                        sprintf(str, "%d inner sequence: size mismatch. Expected %d elements", i, $1_dim1);
                        PyErr_SetString(PyExc_ValueError, str);
                        return NULL;
                }

                for (j = 0; j < $1_dim1; j++) {
                        PyObject *oo = PySequence_GetItem(o,j);
                        
                        for (k = 0; k < $1_dim2; k++) {
                                PyObject *ooo = PySequence_GetItem(oo,k);
                                
                                if (PyNumber_Check(ooo)) {
                                        //temp[i][j][k] = PyInt_AsLong(ooo);
                                        $1[i][j][k] = PyInt_AsLong(ooo);
                                } else {
                                        sprintf(str, "%d inner sequence elements must be numbers", i);      
                                        PyErr_SetString(PyExc_ValueError, str);
                                        return NULL;
                                }
                        }
                }
  }
  //$1 = temp;
*/
}


%enddef


%define ARR3_ARG_CLASS(type)
%typemap(memberin) type [ANY][ANY][ANY] {

        int i, j, k;
        PyObject *o;
        PyObject *oo;
        PyObject *elementObj;

        $result = PyList_New($1_dim0);
        for (i = 0; i < $1_dim0; i++) {
                o = PyList_New($1_dim1);
                PyList_Insert($result, i, o);
                for (j = 0; j < $1_dim1; j++) {
                        oo = PyList_New($1_dim2);
                        PyList_Insert(o, j, oo);
                        for (k = 0; k < $1_dim2; k++) {
                        //elementObj = SWIG_NewPointerObj(&$1[i][j][k], SWIGTYPE_p_xpHashTableContext,0 );
                        elementObj = SWIG_NewPointerObj(&$1[i][j][k], $descriptor(type *),0 );
                        PyList_SetItem(oo, k,  elementObj );
                        }
                }
        }
}

%enddef

ARR_ARG(int)
ARR_ARG(unsigned)
ARR_ARG(signed)
ARR_ARG(unsigned int)
ARR_ARG(signed int)
ARR_ARG(short)
ARR_ARG(unsigned short)
ARR_ARG(signed short)
ARR_ARG(long)
ARR_ARG(unsigned long)
ARR_ARG(signed long)
ARR_ARG(long long)
ARR_ARG(unsigned long long)
ARR_ARG(signed long long)
ARR_ARG(char)
ARR_ARG(unsigned char)
ARR_ARG(signed char)
ARR_ARG(uint8_t)
ARR_ARG(uint16_t)
ARR_ARG(uint32_t)
ARR_ARG(uint64_t)
ARR_ARG(int8_t)
ARR_ARG(int16_t)
ARR_ARG(int32_t)
ARR_ARG(int64_t)
ARR_ARG(uint_fast8_t)
ARR_ARG(uint_fast16_t)
ARR_ARG(uint_fast32_t)
ARR_ARG(uint_fast64_t)
ARR_ARG(int_fast8_t)
ARR_ARG(int_fast16_t)
ARR_ARG(int_fast32_t)
ARR_ARG(int_fast64_t)
ARR_ARG(bigint)
ARR_ARG(xpH1Grp)
ARR_ARG_CLASS(bigint)
ARR_ARG_CLASS(xpCanonCmdEntry)
ARR_ARG_CLASS(xpLayerMapEthernetControl)
ARR_ARG_CLASS(xpKpuTcamSramEntry)
ARR_ARG_CLASS(LAYERCMD_LAYER_CMDS)
ARR_ARG_CLASS(LAYERCMD_HASH_CMDS)
ARR_ARG_CLASS(LAYERCMD_TOKEN_CMDS)
ARR_ARG_CLASS(xpPort2PindexConfig)
ARR_ARG_CLASS(ofitArray)
ARR_ARG_CLASS(kfitArray)

ARR2_ARG(int)
ARR2_ARG(unsigned)
ARR2_ARG(signed)
ARR2_ARG(unsigned int)
ARR2_ARG(signed int)
ARR2_ARG(short)
ARR2_ARG(unsigned short)
ARR2_ARG(signed short)
ARR2_ARG(long)
ARR2_ARG(unsigned long)
ARR2_ARG(signed long)
ARR2_ARG(long long)
ARR2_ARG(unsigned long long)
ARR2_ARG(signed long long)
ARR2_ARG(char)
ARR2_ARG(unsigned char)
ARR2_ARG(signed char)
ARR2_ARG(uint8_t)
ARR2_ARG(uint16_t)
ARR2_ARG(uint32_t)
ARR2_ARG(uint64_t)
ARR2_ARG(int8_t)
ARR2_ARG(int16_t)
ARR2_ARG(int32_t)
ARR2_ARG(int64_t)
ARR2_ARG(uint_fast8_t)
ARR2_ARG(uint_fast16_t)
ARR2_ARG(uint_fast32_t)
ARR2_ARG(uint_fast64_t)
ARR2_ARG(int_fast8_t)
ARR2_ARG(int_fast16_t)
ARR2_ARG(int_fast32_t)
ARR2_ARG(int_fast64_t)

ARR2_ARG_CLASS(xpLpmTableContext)
ARR2_ARG_CLASS(xpControlRegister)


ARR3_ARG(int)
ARR3_ARG(unsigned)
ARR3_ARG(signed)
ARR3_ARG(unsigned int)
ARR3_ARG(signed int)
ARR3_ARG(short)
ARR3_ARG(unsigned short)
ARR3_ARG(signed short)
ARR3_ARG(long)
ARR3_ARG(unsigned long)
ARR3_ARG(signed long)
ARR3_ARG(long long)
ARR3_ARG(unsigned long long)
ARR3_ARG(signed long long)
ARR3_ARG(char)
ARR3_ARG(unsigned char)
ARR3_ARG(signed char)
ARR3_ARG(uint8_t)
ARR3_ARG(uint16_t)
ARR3_ARG(uint32_t)
ARR3_ARG(uint64_t)
ARR3_ARG(int8_t)
ARR3_ARG(int16_t)
ARR3_ARG(int32_t)
ARR3_ARG(int64_t)
ARR3_ARG(uint_fast8_t)
ARR3_ARG(uint_fast16_t)
ARR3_ARG(uint_fast32_t)
ARR3_ARG(uint_fast64_t)
ARR3_ARG(int_fast8_t)
ARR3_ARG(int_fast16_t)
ARR3_ARG(int_fast32_t)
ARR3_ARG(int_fast64_t)
ARR3_ARG(xpHashTableContext)
ARR3_ARG(xpDirectTableContext)
ARR3_ARG_CLASS(xpMatchTableContext)
ARR3_ARG_CLASS(xpIktEntry)
ARR3_ARG_CLASS(uint8_t)
ARR3_ARG_CLASS(xpLayerMapSplitCmdEntry)
ARR3_ARG_CLASS(unsigned char)


////////////////////////////////////////
// INT_ARR(type, num)
//
// setup declaration of array of num integers
// with 
// type()
//
// example:
//
// INT_ARR(MACADDR, 6)
//
// allows to call
//
// >>>addr = MACADDR()
//
// and get integral array of size 6
////////////////////////////////////////
%define INT_ARR(type, num)

	%pythoncode %{

		def type() :
			return [int()] * num

	%}

%enddef


////////////////////////////////////////
// INT_ARR2(type, num0, num1)
// as above for 2-dimensional array of sizes [num0][num1]
////////////////////////////////////////
%define INT_ARR2(type, num0, num1)

	%pythoncode %{

		def type() :
			return [[int()] * num1 for x in xrange(num0)]

	%}

%enddef


////////////////////////////////////////
// any argument of type and declared name:
//
// char* argv[]
//
// will be interpreted as an input array of strings
// array size deduced from the Python command argument line
////////////////////////////////////////
%typemap(in) char* argv[] {
	int size = PySequence_Size($input);
	int i;

	if (!PySequence_Check($input)) {
		PyErr_SetString(PyExc_ValueError,"Expected a sequence");
		return NULL;
	}

	$1 = (char**) malloc(size * sizeof(char*));

	for (i = 0; i < size; i++) {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyString_Check(o)) {
      $1[i] = PyString_AsString(PySequence_GetItem($input, i));
    } else {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be strings");      
      return NULL;
    }
  }

}

%typemap(freearg) char* argv[] {
	if ($1)
		free($1);
}

