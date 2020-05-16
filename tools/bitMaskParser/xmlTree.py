import xml.etree.ElementTree as ET

def main():
	tree = ET.parse('hamada.SVD')
	root = tree.getroot()
	for peripherals in root.iter('peripheral'):
		if str(type(peripherals.find('groupName'))) != '<class \'NoneType\'>':
			peripheralName=peripherals.find('groupName').text
			peripheralFile=open(peripheralName+"_registers.h",'a')
			peripheralFile.write("#ifndef _"+peripheralName+"_REGISTERS_H_\n#define _"+peripheralName+"_REGISTERS_H_\n")
			registerNames=[]
			for registers in peripherals.iter('register'):
				registerName=str(registers.find('name').text)
				registerNames=registerNames+[registerName]
				for fields in registers.iter('field'):
					fieldName=str(fields.find('name').text)
					fieldOffset=int(str(fields.find('bitOffset').text))
					fieldWidth=int(str(fields.find('bitWidth').text))
					if fieldWidth == 1:
						fieldMask=1<<fieldOffset
						peripheralFile.write("\n#define "+peripheralName+"_"+registerName+"_"+fieldName+" ((u32)"+hex(fieldMask)+")")
					else:
						fieldMask=0
						for x in range(fieldWidth):
							fieldMask=(fieldMask|(1<<(x+fieldOffset)))
						fieldMaskComp=0xffffffff&(~fieldMask)
						peripheralFile.write("\n#define "+peripheralName+"_"+registerName+"_"+fieldName+"_SETMASK ((u32)"+hex(fieldMask)+")")
						peripheralFile.write("\n#define "+peripheralName+"_"+registerName+"_"+fieldName+"_CLEARMASK ((u32)"+hex(fieldMaskComp)+")")
						peripheralFile.write("\n#define "+peripheralName+"_"+registerName+"_"+fieldName+"_OFFSET ((u8)"+hex(fieldOffset)+")")
				peripheralFile.write("\n")
			peripheralFile.write("\ntypedef struct "+peripheralName+"type\n{")
			for y in registerNames:
				peripheralFile.write("\n\tu32 "+y+";")
			peripheralFile.write("\n}"+peripheralName+"type;\n\n#endif")
			peripheralFile.close()

if __name__ == "__main__":
    """ This is executed when run from the command line """
    main()