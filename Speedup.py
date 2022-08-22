
import matplotlib.pyplot as plt
import argparse
import subprocess as sp
import json
import re
import math

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

OPFLAGS = ["O0", "O1", "O2", "O3" ]
THREADS = [1, 2, 4, 8, 16]

def PrintFigure(plt, name):
    plt.savefig(name+".svg",format="svg")
    plt.savefig(name+".eps",format="eps")
    plt.savefig(name+".png",format="png")

def parseArgs():
	arg_parser = argparse.ArgumentParser()
	arg_parser.add_argument("-d", "--directory", default="./", help="Specify root directory of project (the directory that contains Naive, Halide, API, etc.")
	arg_parser.add_argument("-o", "--output", default="Speedup", help="Specify output file name.")
	arg_parser.add_argument("-s", "--samples", default=15, help="Specify number of samples taken for each timing experiment.")
	arg_parser.add_argument("-i", "--iterations", default=15, help="Specify number of iterations taken for each sample.")
	arg_parser.add_argument("-mi", "--milliseconds", action="store_true", help="Output timings in milliseconds (default: seconds).")
	arg_parser.add_argument("-sf", "--sig-figs", default=3, help="Output number of significant figures.")
	arg_parser.add_argument("-sh", "--show", action="store_true", help="Render output figures.")
	return arg_parser.parse_args()

def getExecutionTime(inString):
	"""
	@param[in] log      Absolute path to the logfile to be read
	@retval    time     Time of the execution as a float in seconds
	"""
	time  = -1
	error = []
	try:
		stuff = re.findall("Average\srunning\stime\:\s\d+\.\d+",inString)
		if (len(stuff) == 1) and (time < 0):
			numberString = stuff[0].split(": ")[1]
			time = float(numberString)
			return time
		else:
			return -1
	except Exception as e:
		print("Error while trying to read execution time: "+str(e))
		return -1

def retrieveData(args):
	timeMap = { "Naive": {}, "Polly": {}, "Halide": {} }
	try:
		j = json.load( open(args.output+".json", "r") )
		timeMap = j
	except FileNotFoundError:
		print("Could not find data file "+args.output_file+".json. Running collection algorithms...")
		for key in timeMap:
			for op in OPFLAGS:
				timeMap[key][op] = {}
				for thread in THREADS:
					if key == "Polly":
						output = buildProject(op, args, polly=True, threads=thread)
					elif key == "Halide":
						output = buildProject(op, args, halide=True, threads=thread)
					else:
						output = buildProject(op, args, threads=thread)
					if args.milliseconds:
						timeMap[key][op][thread] = getExecutionTime(output)*1000
					else:
						timeMap[key][op][thread] = getExecutionTime(output)
					outputData(timeMap, args)
	return timeMap

def outputData(timeMap, args):
	with open(args.output+".json", "w") as f:
		json.dump(timeMap, f, indent=4)

	with open(args.output+".csv", "w") as f:
		csvString = ""
		for key in timeMap:
			csvString += key+"\n"
			csvString += "OpFlag,"+",".join(str(x) for x in THREADS)+"\n"
			for op in timeMap[key]:
				csvString += op
				for thread in timeMap[key][op]:
					csvString += ","+str(timeMap[key][op][thread])
				csvString += "\n"
			csvString += "\n"
		f.write(csvString)

	with open(args.output+".tex", "w") as f:
		csvString = ""
		for key in timeMap:
			csvString += key+"\n"
			csvString += "OpFlag & "+" & ".join(str(x) for x in THREADS)+" \\\\\n"
			for op in timeMap[key]:
				csvString += op
				for thread in timeMap[key][op]:
					format = '%.'+str(args.sig_figs)+'g'
					csvString += " & "+'%s' % float(format % timeMap[key][op][thread] )
				csvString += " \\\\\n"
			csvString += "\n"
		f.write(csvString)

def buildProject(opflag, args, polly=False, halide=False, threads=1):
	if halide:
		build = "cd Halide ; "
	else:
		build = "cd Naive ; "
	build += "make clean ; "
	if polly:
		build += "make run_polly "
	else:
		build += "make run "
	build += "TIMINGLIB_SAMPLES="+str(args.samples)+" TIMINGLIB_ITERATIONS="+str(args.iterations)+" "
	build += "OPFLAG=-"+opflag+" "
	if polly:
		build += "POLLY_THREADS="+str(threads)+" "
	elif halide:
		build += "HALIDE_THREADS="+str(threads)+" "
	else:
		build += "NUM_THREADS="+str(threads)+" "

	output = ""
	print(build)
	check = sp.Popen( build, stdout=sp.PIPE, stderr=sp.PIPE, shell=True)
	while check.poll() is None:
		output += check.stdout.read().decode("utf-8")
	return output

def plotOpLevel_scatter(normalized, opLevel, args):
	fig = plt.figure(figsize=figDim, dpi=figDPI, frameon=False)
	ax = fig.add_subplot(1, 1, 1, frameon=False)
	xticklabels = THREADS
	naive = ax.scatter(THREADS, [x for x in normalized["Naive"][opLevel].values()], label="Naive")
	polly = ax.scatter(THREADS, [x for x in normalized["Polly"][opLevel].values()], label="Polly")
	halide = ax.scatter(THREADS, [x for x in normalized["Halide"][opLevel].values()], label="Halide")
	ax.set_title(opLevel+" Speedup", fontsize=titleFont)
	ax.set_xlabel("Threads", fontsize=axisFont)
	plt.xscale("log", base=2)
	ax.set_ylabel("Speedup", fontsize=axisFont)
	plt.yscale("log", base=2)
	# draw dashed lines one each of the y-axis ticks
	ymax   = -1000000
	ymin   =  1000000
	yticks = []
	for type in normalized:
		for thread in normalized[type][opLevel]:
			if math.log2(normalized[type][opLevel][thread]) > ymax:
				ymax = math.log2(normalized[type][opLevel][thread])
			if math.log2(normalized[type][opLevel][thread]) < ymin:
				ymin = math.log2(normalized[type][opLevel][thread])
	for exp in range(math.floor(ymin), math.ceil(ymax)+1):
		if (exp % 2) == 0:
			plt.axhline(y=2**exp, color="black", linestyle="dashed")
	ax.legend(frameon=False)
	PrintFigure(plt, opLevel+"_"+args.output)

def plotOpLevel_bar(normalized, opLevel, args):
	fig = plt.figure(figsize=figDim, dpi=figDPI, frameon=False)
	ax = fig.add_subplot(1, 1, 1, frameon=False)
	xticklabels = THREADS
	naive = ax.bar([x - barWidth for x in range(len(xticklabels))], [x for x in normalized["Naive"][opLevel].values()], barWidth, label="Naive")
	polly = ax.bar([x for x in range(len(xticklabels))], [x for x in normalized["Polly"][opLevel].values()], barWidth, label="Polly")
	halide = ax.bar([x + barWidth for x in range(len(xticklabels))], [x for x in normalized["Halide"][opLevel].values()], barWidth, label="Halide")
	ax.set_title(opLevel+" Speedup", fontsize=titleFont)
	ax.set_ylabel("Speedup", fontsize=axisFont)
	ax.set_xlabel("Threads", fontsize=axisFont)
	ax.set_ylim(bottom=0)
	ax.legend(frameon=False)
	# ax.bar_label(naive, padding=3) -> not available until matplotlib 3.4
	plt.xticks(ticks=[x for x in range(len(xticklabels))], labels=xticklabels, fontsize=axisFont, rotation=xtickRotation)
	PrintFigure(plt, opLevel+"_"+args.output)

def plotNormalizedSpeedup(timeMap, args, scatter=True, bar=False):
	"""
	All times are normalized by the polly -O3 implementation
	"""
	normalized = {}
	for type in timeMap:
		normalized[type] = {}
		for oplevel in timeMap[type]:
			normalized[type][oplevel] = {} 
			for thread in timeMap[type][oplevel]:
				normalized[type][oplevel][thread] = timeMap["Polly"]["O3"]["1"] /timeMap[type][oplevel][thread]
	for op in OPFLAGS: 
		if scatter:
			plotOpLevel_scatter(normalized, op, args)
		elif bar:
			plotOpLevel_bar(normalized, op, args)
	if args.show:
		plt.show()

args = parseArgs()
timeMap = retrieveData(args)
plotNormalizedSpeedup(timeMap, args)
