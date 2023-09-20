
import matplotlib.pyplot as plt
import argparse
import subprocess as sp
import json
import re
import math
import os

# plot parameters
figDim = (6,6) # in inches
figDPI = 100 # creates 200x200 image
axisFont  = 10
axisLabelFont  = 10
titleFont = 16
xtickRotation = 0
colors = [
           ( 255./255,  95./255,  95./255, 255./255 ), # HC, orangish red
           (  50./255, 162./255,  81./255, 255./255 ), # HL, leaf green
           ( 190./255,  10./255, 255./255, 255./255 ), # PaMul, violet
           ( 255./255, 153./255,  51./255, 255./255 ), # HCHL, lite brown
           ( 255./255, 102./255, 178./255, 255./255 ), # HCPaMul, pink
           (  51./255, 153./255, 255./255, 255./255 ), # HLPaMul, sky blue
           ( 153./255, 153./255, 255./255, 255./255 ), # HCHLPaMul, brown-purple 
           ( 255./255, 178./255, 100./255, 255./255 ), # None, tan
           ( 121./255, 154./255, 134./255, 255./255 ), # olive green
           ( 198./255, 195./255,  71./255, 255./255 ), # mustard yellow
           ( 204./255, 153./255, 255./255, 255./255 )  # light violet
         ]
markers = [ 'o', '^', '1', 's', '*', 'd', 'X', '>']
barWidth = 0.3

# data file to be printed
complianceFileName = "Compliance.json"
# install prefix of Algorithms repo
rootPath = "/home/ben/Documents/Research/Algorithms/"
# folders to find and build within them
buildFolders = { "PERFECT","Naive" }
# set the opflag to what you want to test
#OPFLAGS = ["O0", "O1", "O2", "O3" ]
OPFLAGS = ["O1"]

def PrintFigure(plt, name):
    plt.savefig(name+".svg",format="svg")
    plt.savefig(name+".eps",format="eps")
    plt.savefig(name+".png",format="png")

def parseArgs():
	arg_parser = argparse.ArgumentParser()
	arg_parser.add_argument("-d", "--directory", default="./", help="Specify root directory of project (the directory that contains Naive, Halide, API, etc.")
	arg_parser.add_argument("-o", "--output", default="Compliance", help="Specify output file name.")
	arg_parser.add_argument("-s", "--samples", default=15, help="Specify number of samples taken for each timing experiment.")
	arg_parser.add_argument("-i", "--iterations", default=15, help="Specify number of iterations taken for each sample.")
	arg_parser.add_argument("-mi", "--milliseconds", action="store_true", help="Output timings in milliseconds (default: seconds).")
	arg_parser.add_argument("-sf", "--sig-figs", default=3, help="Output number of significant figures.")
	arg_parser.add_argument("-sh", "--show", action="store_true", help="Render output figures.")
	return arg_parser.parse_args()

def getErrors(inString):
	"""
	@param[in] log      Absolute path to the logfile to be read
	@retval    time     Time of the execution as a float in seconds
	"""
	errors = {}
	try:
		stuff = re.findall("\[critical\]\s.*", inString)
		success = re.findall("\[info\]\sGrammar\sSuccess", inString)
		for error in stuff:
			if errors.get(error) is None:
				errors[error] = 1
			else:
				errors[error] += 1
		errors["Success"] = len(success)
		return errors
	except Exception as e:
		print("Error while parsing execution log: "+str(e))
		return -1

def outputData(complianceMap, path, op):
	# implement Total category
	printMap = {}
	total = dict.fromkeys(OPFLAGS, {"Total Errors": 0, "Success": 0, "Compliance": 0.0})
	for path in complianceMap:
		printMap[path] = {}
		for op in complianceMap[path]:
			printMap[path][op] = {}
			for error in complianceMap[path][op]:
				prettyError = error.split(": ")[-1]
				printMap[path][op][prettyError] = complianceMap[path][op][error]
				if total[op].get(prettyError) is None:
					total[op][prettyError] = 0
				total[op][prettyError] += complianceMap[path][op][error]
				if error != "Success":
					total[op]["Total Errors"] += complianceMap[path][op][error]
			total[op]["Compliance"] = ( ( total[op]["Success"] / (total[op]["Total Errors"] + total[op]["Success"]) )\
										if (total[op]["Total Errors"] + total[op]["Success"]) else 0.0 ) * 100
	# per-project total
	for path in complianceMap:
		projectName = path.split("/")[-1]
		if total.get(projectName) is None:
			total[projectName] = {}
		for op in complianceMap[path]:
			if total[projectName].get(op) is None:
				total[projectName][op] = {}
				total[projectName][op]["Total Errors"] = 0
				total[projectName][op]["Success"] = 0
				total[projectName][op]["Compliance"] = 0.0
			for error in complianceMap[path][op]:
				prettyError = error.split(": ")[-1]
				if total[projectName][op].get(prettyError) is None:
					total[projectName][op][prettyError] = 0
				total[projectName][op][prettyError] += complianceMap[path][op][error]
				if error != "Success":
					total[projectName][op]["Total Errors"] += complianceMap[path][op][error]
			total[projectName][op]["Compliance"] = ( ( total[projectName][op]["Success"] / \
													 (total[projectName][op]["Total Errors"] + total[projectName][op]["Success"]) )\
										             if (total[projectName][op]["Total Errors"] + total[projectName][op]["Success"])\
													 else 0.0 ) * 100
		
	printMap["Total"] = total
	
			
	with open(complianceFileName, "w") as f:
		json.dump(printMap, f, indent=2)

def buildProject(path, opflag, args, polly=False, api = False, halide=False, PERF=False):
	build = "cd "+path+" ; make clean ; make OPFLAG=-"+opflag
	output = ""
	print(build)
	check = sp.Popen( build, stdout=sp.PIPE, stderr=sp.PIPE, shell=True)
	while check.poll() is None:
		output += check.stdout.read().decode("utf-8")
	return output

def callProject(complianceMap, path, op, args):
	output = ""
	output = buildProject(path, op, args)
	complianceMap[path][op] = getErrors(output)
	outputData( complianceMap, path, op )

def findOffset(path, basePath):
	b = set(basePath.split("/"))
	b.remove("")
	p = set(path.split("/"))
	p.remove("")
	offset = p - p.intersection(b)
	# now reconstruct the ordering
	orderedOffset = []
	while len(offset) > 0:
		for entry in path.split("/"):
			if entry in offset:
				orderedOffset.append(entry)
				offset.remove(entry)
	return "/".join(x for x in orderedOffset)

def recurseIntoFolder(path, BuildNames, basePath, folderMap):
	"""
	@brief 	   recursive algorithm to search through all branches of a directory tree for directories containing files of interest

	In general we want a function that can search through a tree of directories and find build folders we are interested in
	The resulting map will form the foundation for uniquifying each entry of data (likely from and automation flow)
	This function constructs that uniquifying map, and at each leaf there lies a build folder of interest

	@param[in] path			Absolute path to a directory of interest. Initial call should be to to the root of the tree to be searched
	@param[in] BuildNames	Folder names that are being sought after. Likely build folder names
	@param[in] folderMap      Hash of the data being processed. Contains [project][logFileName][Folder]
	"""
	BuildNames = set(BuildNames)
	currentFolder = path.split("/")[-1]
	path += "/"
	offset = findOffset(path, basePath)
	
	if currentFolder in BuildNames:
		if folderMap.get(offset) is None:
			folderMap[offset] = dict()
	directories = []
	for f in os.scandir(path):
		if f.is_dir():
			directories.append(f)

	for d in directories:
		folderMap = recurseIntoFolder(d.path, BuildNames, basePath, folderMap)
	
	return folderMap

def buildAndCollectData(rootFolder, buildFolders, args):
	# contains paths to all directories that contain files we seek 
	# project path : opflag : error map
	complianceMap = {}
	priorResults  = {}
	try:
		with open(complianceFileName, "r") as f:
			priorResults = json.load(f)
	except FileNotFoundError:
		print("No pre-existing log info file. Running collection algorithm...")
	# determines if the data generation code needs to be run
	recurseIntoFolder(rootFolder, buildFolders, rootFolder, complianceMap)
	# now inject opflags into each entry and any prior results
	for path in complianceMap:
		for op in OPFLAGS:
			complianceMap[path][op] = {}
			if priorResults.get(path) is not None:
				if priorResults[path].get(op) is not None:
					complianceMap[path][op] = priorResults[path][op]

	# now we build each project, skipping the projects in priorResults
	for entry in complianceMap:
		for op in complianceMap[entry]:
			if len(complianceMap[entry][op]) == 0:
				callProject(complianceMap, entry, op, args)

	return complianceMap

args = parseArgs()
compliance = buildAndCollectData(rootPath, buildFolders, args)
