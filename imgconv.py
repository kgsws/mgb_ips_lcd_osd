#!/usr/bin/python3
import sys
from os import listdir
from os import path
from PIL import Image

def get_files(dirname):
	filelist = [f for f in listdir(dirname) if path.isfile(path.join(dirname, f)) and path.splitext(f)[1] == ".png"]
	return filelist

def dump_buffer(f, data, name):
	f.write("static __code const uint8_t %s[] =\n{\n\t" % name)
	for b in data:
		f.write("0x%02X," % b)
	f.write("\n};\n\n")

# check arguments
if len(sys.argv) != 3:
	print("usage:", sys.argv[0], "input_dir output")
	exit(1)

# get image list
files = get_files(sys.argv[1])
if len(files) <= 0:
	print("No images found!")
	exit(1)

# open output file
f = open(sys.argv[2], "w")
f.write("// GENERATED FILE; DO NOT MODIFY\n// YOUR CHANGES WILL BE LOST!\n\n")

# parse images
for name in files:
	file = path.join(sys.argv[1], name)
	if name == "logo.png":
		img = Image.open(file).convert("L")
		if img.width != 160 or img.height != 52:
			raise Exception("Invalid logo resolution!")
		pix = img.getdata()
		data = bytearray()
		for i in range(0, 160 * 52, 8):
			color = pix[i] >> 7
			color |= (pix[i+1] >> 6) & 0x02
			color |= (pix[i+2] >> 5) & 0x04
			color |= (pix[i+3] >> 4) & 0x08
			color |= (pix[i+4] >> 3) & 0x10
			color |= (pix[i+5] >> 2) & 0x20
			color |= (pix[i+6] >> 1) & 0x40
			color |= pix[i+7] & 0x80
			data.append(color)
		dump_buffer(f, data, path.splitext(name)[0])
	else:
		pix = Image.open(file).convert("L")
		width = pix.width + 4 + 3 # 4 for edges, 3 for padding
		width &= 0xFC
		height = pix.height + 2 # 2 for edges
		img = Image.new("L", (width, height))
		img.paste(pix, ((width - pix.width) // 2, 1))
		pix = img.getdata()
		data = bytearray()
		size = width * height
		data.append(width)
		data.append(height)
		for i in range(0, size, 4):
			color = pix[i] >> 6
			color |= (pix[i+1] & 0xC0) >> 4
			color |= (pix[i+2] & 0xC0) >> 2
			color |= pix[i+3] & 0xC0
			data.append(color)
		dump_buffer(f, data, path.splitext(name)[0])

# done
f.close()
