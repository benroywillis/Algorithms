
import json

# this script will generate the 7 correspondence categories (HC, HL, Cb, HCHL, HCCb, HLCb, HCHLCb) and map the source code lines to them

# quick little helper turns sets into lists when serializing jsons
class SetEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, set):
            return list(obj)
        return json.JSONEncoder.default(self, obj)

sourceMapFile = "SourceMap_StencilChain_kernel.json"
HCFile = "kernel_StencilChain.json_HotCode.json"
HLFile = "kernel_StencilChain.json_HotLoop.json"
CbFile = "instance_StencilChain.json"

HCkernels = json.load( open(HCFile, "r") )
HLkernels = json.load( open(HLFile, "r") )
Cbkernels = json.load( open(CbFile, "r") )

HCblocks = set()
for k in HCkernels["Kernels"]:
	if HCkernels["Kernels"][k].get("Blocks") is None:
		continue
	for b in HCkernels["Kernels"][k]["Blocks"]:
		HCblocks.add(int(b))

HLblocks = set()
for k in HLkernels["Kernels"]:
	if HLkernels["Kernels"][k].get("Blocks") is None:
		continue
	for b in HLkernels["Kernels"][k]["Blocks"]:
		HLblocks.add(int(b))

Cbblocks = set()
for k in Cbkernels["Kernels"]:
	if Cbkernels["Kernels"][k].get("Blocks") is None:
		continue
	for b in Cbkernels["Kernels"][k]["Blocks"]:
		Cbblocks.add(int(b))

# generate exclusivity sets
HC = HCblocks - HLblocks - Cbblocks
HL = HLblocks - HCblocks - Cbblocks
Cb = Cbblocks - HCblocks - HLblocks
HCHL = HCblocks.intersection( HLblocks ) - Cbblocks
HCCb = HCblocks.intersection( Cbblocks ) - HLblocks
HLCb = HLblocks.intersection( Cbblocks ) - HCblocks
HCHLCb = HCblocks.intersection( HLblocks ).intersection( Cbblocks )

# map source code lines to exclusivity sets
exclusivityMap = { "HC": {}, "HL": {} , "Cb": {}, "HCHL": {}, "HCCb": {}, "HLCb": {}, "HCHLCb": {} }
sourceMap = json.load( open(sourceMapFile, "r") )
for block in sourceMap["Blocks"]:
	if int(block) in HC:
		for entry in sourceMap["Blocks"][block]:
			if exclusivityMap["HC"].get(entry) is None:
				exclusivityMap["HC"][entry] = set()
			for b in sourceMap["Blocks"][block][entry]:
				exclusivityMap["HC"][entry].add(b)
	elif int(block) in HL:
		for entry in sourceMap["Blocks"][block]:
			if exclusivityMap["HL"].get(entry) is None:
				exclusivityMap["HL"][entry] = set()
			for b in sourceMap["Blocks"][block][entry]:
				exclusivityMap["HL"][entry].add(b)
	elif int(block) in Cb:
		for entry in sourceMap["Blocks"][block]:
			if exclusivityMap["Cb"].get(entry) is None:
				exclusivityMap["Cb"][entry] = set()
			for b in sourceMap["Blocks"][block][entry]:
				exclusivityMap["Cb"][entry].add(b)
	elif int(block) in HCHL:
		for entry in sourceMap["Blocks"][block]:
			if exclusivityMap["HCHL"].get(entry) is None:
				exclusivityMap["HCHL"][entry] = set()
			for b in sourceMap["Blocks"][block][entry]:
				exclusivityMap["HCHL"][entry].add(b)
	elif int(block) in HCCb:
		for entry in sourceMap["Blocks"][block]:
			if exclusivityMap["HCCb"].get(entry) is None:
				exclusivityMap["HCCb"][entry] = set()
			for b in sourceMap["Blocks"][block][entry]:
				exclusivityMap["HCCb"][entry].add(b)
	elif int(block) in HLCb:
		for entry in sourceMap["Blocks"][block]:
			if exclusivityMap["HLCb"].get(entry) is None:
				exclusivityMap["HLCb"][entry] = set()
			for b in sourceMap["Blocks"][block][entry]:
				exclusivityMap["HLCb"][entry].add(b)
	elif int(block) in HCHLCb:
		for entry in sourceMap["Blocks"][block]:
			if exclusivityMap["HCHLCb"].get(entry) is None:
				exclusivityMap["HCHLCb"][entry] = set()
			for b in sourceMap["Blocks"][block][entry]:
				exclusivityMap["HCHLCb"][entry].add(b)
	# else it was not structured, move on

with open("ExclusivityMap.json","w") as f:
	sortedMap = { "HC": {}, "HL": {}, "Cb": {}, "HCHL": {}, "HCCb": {}, "HLCb": {}, "HCHLCb": {} }
	for key in exclusivityMap:
		for file in sorted(exclusivityMap[key]):
			sortedMap[key][file] = []
			for block in sorted(exclusivityMap[key][file]):
				sortedMap[key][file].append(block)
	json.dump(sortedMap, f, indent=2, cls=SetEncoder)
