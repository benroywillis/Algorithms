
import matplotlib.pyplot as plt
import argparse
import subprocess as sp
import json
import re
import math
import os
import datetime
import copy

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

# install prefix of Algorithms repo
rootPath = "/home/ben/Documents/Research/Algorithms/"
# folders to find and build within them
#targetFolders = { "PERFECT","Naive","MachSuite" }
targetFolders = { "MachSuite" }
# set the opflag to what you want to test
#OPFLAGS = ["O0", "O1", "O2", "O3" ]
OPFLAGS = ["O1"]

def PrintFigure(plt, name):
	plt.savefig(name+".svg",format="svg")
	plt.savefig(name+".eps",format="eps")
	plt.savefig(name+".png",format="png")

def parseArgs():
	arg_parser = argparse.ArgumentParser()
	arg_parser.add_argument("-i", "--input", default="Compliance", help="Specify input data file name. Defaults to Compliance_yyyy-mm-dd.json")
	arg_parser.add_argument("--full", action="store_true", help="Pass this flag to run the entire Cyclebite toolchain on each project. Default behavior is to remove only the previously existing KernelGrammar file.")
	arg_parser.add_argument("--debug", default="-g3", help="Set debug flag for compile commands. This raw argument will be based to the DEBUG macro in the build script.")
	arg_parser.add_argument("-o", "--output", default="Compliance", help="Specify output file name. Defaults to Compliance_yyyy-mm-dd.json.")
	args = arg_parser.parse_args()
	if args.input  == "Compliance":
		args.input  = "Compliance_"+datetime.datetime.today().strftime("%Y-%m-%d")+".json"
	if args.output == "Compliance":
		args.output = "Compliance_"+datetime.datetime.today().strftime("%Y-%m-%d")+".json"
	return args

def getCompliance(inString):
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

def getLabels(inString):
	labels = {}
	try:
		stuff = re.findall("\[info\]\sCyclebite-Template\sLabel\:\s.*", inString)
		for entry in stuff:
			localizedTask  = re.findall("Task\d+", entry)[0]
			localizedLabel = re.findall("\-\>\s\w+", entry)[0][3:]
			if labels.get(localizedTask) is None:
				labels[localizedTask] = []
			labels[localizedTask].append(localizedLabel)
		return labels
	except Exception as e:
		print("Error while parsing execution log: "+str(e))
		return -1

def outputData(complianceMap, path, op):
	printMap = {}
	# write results to output dict and implement Total category
	total = dict.fromkeys(OPFLAGS, {"Total Errors": 0, "Success": 0, "Compliance": 0.0})
	for path in complianceMap:
		printMap[path] = {}
		for op in complianceMap[path]:
			printMap[path][op] = { "Labels": {}, "Results":{} }
			printMap[path][op]["Labels"] = complianceMap[path][op]["Labels"]
			for error in complianceMap[path][op]["Results"]:
				prettyError = error.split(": ")[-1]
				printMap[path][op]["Results"][prettyError] = complianceMap[path][op]["Results"][error]
				if total[op].get(prettyError) is None:
					total[op][prettyError] = 0
				total[op][prettyError] += complianceMap[path][op]["Results"][error]
				if error != "Success":
					total[op]["Total Errors"] += complianceMap[path][op]["Results"][error]
			total[op]["Compliance"] = ( ( total[op]["Success"] / (total[op]["Total Errors"] + total[op]["Success"]) )\
										if (total[op]["Total Errors"] + total[op]["Success"]) else 0.0 ) * 100
	## label correctness
	# this map holds the correct labels for all known tasks in the corpus ( { oplevel: { appPath: { label: [tasks] } } } )
	labelKey = {}
	# this map reverse-maps labelKey ( { oplevel: { appPath: { task: [labels] } } } )
	actualTaskToLabel = {}
	with open("LabelKey.json","r") as f:
		labelKey = json.load( f )
	for path in complianceMap:
		for op in complianceMap[path]:
			if actualTaskToLabel.get(op) is None:
				actualTaskToLabel[op] = {}
			actualTaskToLabel[op][path] = {}
			# this maps tasks to their labels
			if labelKey[op].get(path) is None:
				print("Warning: path "+path+" was not found in LabelKey.json")
				continue
			for label in labelKey[op][path]:
				for task in labelKey[op][path][label]:
					if actualTaskToLabel[op][path].get(task) is None:
						actualTaskToLabel[op][path][task] = []
					actualTaskToLabel[op][path][task].append(label)

	for path in complianceMap:
		for op in complianceMap[path]:
			if total[op].get("Labels") is None:
				total[op]["Labels"] = { "Correct": {}, "Incorrect": {} }
			printMap[path][op]["Labels"]["Correct"] = {}
			printMap[path][op]["Labels"]["Incorrect"] = {}
	for path in complianceMap:
		for op in complianceMap[path]:
			if labelKey.get(op) is not None:
				if labelKey[op].get(path) is not None:
					# this loop determines the false-positive and false-negative count for each label
					for task in complianceMap[path][op]["Labels"]:
						if (task == "Correct") or (task == "Incorrect"):
							continue
						if actualTaskToLabel[op][path].get(task) is None:
							actualTaskToLabel[op][path][task] = []
						wrongLabels = { task: {} }
						for predictedLabel in complianceMap[path][op]["Labels"][task]:
							if predictedLabel in actualTaskToLabel[op][path][task]:
								# correct
								if total[op]["Labels"]["Correct"].get(predictedLabel) is None:
									total[op]["Labels"]["Correct"][predictedLabel] = 0
								total[op]["Labels"]["Correct"][predictedLabel] += 1
								if printMap[path][op]["Labels"]["Correct"].get(predictedLabel) is None:
									printMap[path][op]["Labels"]["Correct"][predictedLabel] = 0
								printMap[path][op]["Labels"]["Correct"][predictedLabel] += 1
							else:
								# incorrect
								if wrongLabels.get(task) is None:
									wrongLabels[task] = {}
								wrongLabels[task][predictedLabel] = actualTaskToLabel[op][path][task]
						for predictedLabel in wrongLabels[task]:
							if printMap[path][op]["Labels"]["Incorrect"].get(task) is None:
								printMap[path][op]["Labels"]["Incorrect"][task] = { predictedLabel: "" }
							printMap[path][op]["Labels"]["Incorrect"][task][predictedLabel] = " or ".join( x for x in  wrongLabels[task][predictedLabel] )
							if total[op]["Labels"]["Incorrect"].get(predictedLabel) is None:
								total[op]["Labels"]["Incorrect"][predictedLabel] = []
							total[op]["Labels"]["Incorrect"][predictedLabel].append( " or ".join( x for x in wrongLabels[task][predictedLabel] ) )
						
	# per-project total
	for path in complianceMap:
		projectName = list(set( [x for x in path.split("/")] ).intersection(targetFolders))[0]
		if total.get(projectName) is None:
			total[projectName] = {}
		for op in complianceMap[path]:
			if total[projectName].get(op) is None:
				total[projectName][op] = {}
				total[projectName][op]["Total Errors"] = 0
				total[projectName][op]["Success"] = 0
				total[projectName][op]["Compliance"] = 0.0
			for error in complianceMap[path][op]["Results"]:
				prettyError = error.split(": ")[-1]
				if total[projectName][op].get(prettyError) is None:
					total[projectName][op][prettyError] = 0
				total[projectName][op][prettyError] += complianceMap[path][op]["Results"][error]
				if error != "Success":
					total[projectName][op]["Total Errors"] += complianceMap[path][op]["Results"][error]
			total[projectName][op]["Compliance"] = ( ( total[projectName][op]["Success"] / \
													 (total[projectName][op]["Total Errors"] + total[projectName][op]["Success"]) )\
										             if (total[projectName][op]["Total Errors"] + total[projectName][op]["Success"])\
													 else 0.0 ) * 100
	printMap["Total"] = total

	with open(args.output, "w") as f:
		json.dump(printMap, f, indent=2)

def buildProject(path, opflag, args, polly=False, api = False, halide=False, PERF=False):
	if args.full:
		build = "cd "+path+" ; make clean ; make OPFLAG=-"+opflag+" DEBUG="+args.debug
	else:
		build = "cd "+path+" ; rm KernelGrammar* ; make OPFLAG=-"+opflag+" DEBUG="+args.debug
	output = ""
	print(build)
	check = sp.Popen( build, stdout=sp.PIPE, stderr=sp.PIPE, shell=True)
	while check.poll() is None:
		output += check.stdout.read().decode("utf-8")
	return output

def callProject(complianceMap, path, op, args):
	output = ""
	output = buildProject(path, op, args)
	complianceMap[path][op]["Results"] = getCompliance(output)
	complianceMap[path][op]["Labels"] = getLabels(output)
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
	#if currentFolder in BuildNames:
	if "Makefile" in set( [f for f in os.listdir(path) if os.path.isfile( path+f )] ):
		if len( set( [x for x in path.split("/")] ).intersection(BuildNames) ):
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
		with open(args.input, "r") as f:
			priorResults = json.load(f)
	except FileNotFoundError:
		print("No pre-existing log info file. Running collection algorithm...")
	# determines if the data generation code needs to be run
	recurseIntoFolder(rootFolder, buildFolders, rootFolder, complianceMap)
	# now inject opflags into each entry and any prior results
	for path in complianceMap:
		for op in OPFLAGS:
			complianceMap[path][op] = { "Labels": {}, "Results": {} }
			if priorResults.get(path) is not None:
				if priorResults[path].get(op) is not None:
					complianceMap[path][op] = priorResults[path][op]

	# now we build each project, skipping the projects in priorResults
	for entry in complianceMap:
		for op in complianceMap[entry]:
			if len(complianceMap[entry][op]["Results"]) == 0:
				callProject(complianceMap, entry, op, args)

	return complianceMap

args = parseArgs()
compliance = buildAndCollectData(rootPath, targetFolders, args)
