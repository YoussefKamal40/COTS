import sys
import re

def main():
	xmlName = input("Please enter xml file name:\n")
	xmlFile=open(str(xmlName),'r')
	xmlAllLines=xmlFile.readlines()
	xmlAllLinesInOne="";
	for x in xmlAllLines:
		xmlAllLinesInOne=xmlAllLinesInOne+x
	peripherals = re.findall(r'(<peripheral>([\w\W]+?)</peripheral>)',xmlAllLinesInOne)
	for x in peripherals:
		peripheralName=str(re.search(r'(<groupName>([\w\W]+?)</groupName>)',x[1])[2])
		peripheralFile=open(peripheralName+"_registers.h",'a')
		peripheralFile.write("#ifndef _"+peripheralName+"_REGISTERS_H_\n#define _"+peripheralName+"_REGISTERS_H_\n")
		registers = re.findall(r'(<register>([\w\W]+?)</register>)',x[1])
		registerNames=[]
		for y in registers:
			registerName=str(re.search(r'(<name>([\w\W]+?)</name>)',y[1])[2])
			registerNames=registerNames+[registerName]
			feilds=re.findall(r'(<field>([\w\W]+?)</field>)',y[1])
			for z in feilds:
				feildName=str(re.search(r'(<name>([\w\W]+?)</name>)',z[1])[2])
				feildOffset=int(str(re.search(r'(<bitOffset>([\w\W]+?)</bitOffset>)',z[1])[2]))
				feildWidth=int(str(re.search(r'(<bitWidth>([\w\W]+?)</bitWidth>)',z[1])[2]))
				if feildWidth == 1:
					feildMask=1<<feildOffset
					peripheralFile.write("\n#define "+peripheralName+"_"+registerName+"_"+feildName+" ((u32)"+hex(feildMask)+")")
				else:
					feildMask=0
					for x in range(feildWidth):
						feildMask=(feildMask|(1<<(x+feildOffset)))
					feildMaskComp=0xffffffff&(~feildMask)
					peripheralFile.write("\n#define "+peripheralName+"_"+registerName+"_"+feildName+"_SETMASK ((u32)"+hex(feildMask)+")")
					peripheralFile.write("\n#define "+peripheralName+"_"+registerName+"_"+feildName+"_CLEARMASK ((u32)"+hex(feildMaskComp)+")")
					peripheralFile.write("\n#define "+peripheralName+"_"+registerName+"_"+feildName+"_OFFSET ((u8)"+hex(feildOffset)+")")
			peripheralFile.write("\n")	
		peripheralFile.write("\ntypedef struct "+peripheralName+"type\n{")
		for y in registerNames:
			peripheralFile.write("\n\tu32 "+y+";")
		peripheralFile.write("\n}"+peripheralName+"type;\n\n#endif")
		peripheralFile.close()
	xmlFile.close()

if __name__ == "__main__":
    """ This is executed when run from the command line """
    main()