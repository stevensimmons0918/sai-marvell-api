import os
import sys
primitiveType = ["char", "int", "unsigned", "signed", "float", "double", "short", "long", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t", "int32_t", "int64_t" , "uint","bool"]

def structArray(targetName):
	findStruct = 0
	genHfileName = './SWIG/'+targetName+'.gen.h'
	if(os.path.isfile(genHfileName)):
		genHfp = open(genHfileName, 'rw+')
	else:
		exit(1)
	genHFileLines = genHfp.readlines()

	modifiedIfileName = "./SWIG/" + targetName +"_wrap_modified.cpp"
	if(os.path.isfile(modifiedIfileName)):
		modifiedIfp = open(modifiedIfileName, 'rw+')
	else:
		exit(1)

	for index, line in enumerate(genHFileLines):
		dataType = line.split(" ")[3]
		modifiedIfp.seek(0,0)
		if(dataType not in primitiveType):
			bufLine = line
			modifiedIfileLines = modifiedIfp.readlines()
			for lineIndex, fileLine in enumerate(modifiedIfileLines):
				if(line == fileLine):
					modifiedIfileLines[lineIndex] = ''

				structEndLine = '}' + dataType + ';\n'
				tmpBuf = fileLine.replace(" ","")
				if(tmpBuf == structEndLine):
					modifiedIfileLines[lineIndex] = fileLine + line

				structStartLine = 'struct'+dataType+'{\n'
				if(tmpBuf == structStartLine):
					findStruct = 1

				structEndLine = '};\n'
				if(tmpBuf == structEndLine and findStruct == 1):
					modifiedIfileLines[lineIndex] = fileLine + line
					findStruct = 0

				structStartLine = 'struct'+dataType+'\n'
				if(tmpBuf == structStartLine):
					findStruct = 1

				structEndLine = '};\n'
				if(tmpBuf == structEndLine and findStruct == 1):
					modifiedIfileLines[lineIndex] = fileLine + line
					findStruct = 0

				structStartLine = 'typedef int64_t ' + dataType +';\n'
				if(structStartLine in fileLine):
					modifiedIfileLines[lineIndex] = fileLine + line

				structStartLine = 'typedef long long ' + dataType +';\n'
				if(structStartLine in fileLine):
					modifiedIfileLines[lineIndex] = fileLine + line

				structStartLine = 'typedef uint8_t ' + dataType +';\n'
				if(structStartLine in fileLine):
					modifiedIfileLines[lineIndex] = fileLine + line
				
				structStartLine = 'typedef uint32_t ' + dataType +';\n'
				if(structStartLine in fileLine):
					modifiedIfileLines[lineIndex] = fileLine + line

				structStartLine = 'typedef uint64_t ' + dataType +';\n'
				if(structStartLine in fileLine):
					modifiedIfileLines[lineIndex] = fileLine + line

				structStartLine = 'typedef uint16_t ' + dataType +';\n'
				if(structStartLine in fileLine):
					modifiedIfileLines[lineIndex] = fileLine + line

				#structStartLine = 'typedef unsigned int ' + dataType +';\n'
				#if(structStartLine in fileLine):
				#	modifiedIfileLines[lineIndex] = fileLine + line

			modifiedIfp.seek(0,0)
			line = modifiedIfp.writelines(modifiedIfileLines)

	modifiedIfp.close()
	genHfp.close()

if __name__ == "__main__":
        if( len( sys.argv ) != 2 ):
           exit( 1 )
        structArray(sys.argv[ 1 ])
