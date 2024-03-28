import matplotlib.pyplot as plt
import json
import argparse
import math

# plot parameters
figDim = (30, 12) # in inches
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

Apps = { "ElementMultiply (SIZE=2048)", "GEMV (SIZE=2048)", "SGEMM (SIZE=512)", "Stencil Chain (1280x1920)", "polybench/DGEMM (SIZE=1024)", "polybench/3mm (SIZE=1024)", "polybench/gemver (SIZE=4096)"  }
THREADS=[1,2,4,8,16]

def PrintFigure(plt, name):
    plt.savefig(name+".svg",format="svg")
    plt.savefig(name+".eps",format="eps")
    plt.savefig(name+".png",format="png")

def parseArgs():
	arg_parser = argparse.ArgumentParser()
	arg_parser.add_argument("-i", "--input", default="CyclebiteSpeedupData.json", help="Specify input data file name.")
	arg_parser.add_argument("-o", "--output", default="CyclebiteTemplateSpeedup", help="Specify output figure name.")
	arg_parser.add_argument("-sh", "--show", action="store_true", help="Render output figures.")
	return arg_parser.parse_args()

def retrieveData(args):
	"""
	input data should be organized as:
	{
		"App": {
			"Type (e.g., Cyclebite-Template)": {
				"1": <time>,
				"2": <time>,
				"4": <time>,
				"8": <time>,
				"16": <time>
			},
			...
		},
		...
	}
	"""
	j = json.load( open(args.input, "r") )
	timeMap = {}
	for key in j:
		if key in Apps:
			timeMap[key] = j[key]
	return timeMap

def plotOpLevel_scatter(data, args):
    fig, axes = plt.subplots(1, len(data.keys()), sharey=True, figsize=figDim, dpi=figDPI, frameon=False)

	# normalize data by single-thread LLVM-Polly performance
    normalized = {}
    for app in data:
        normalized[app] = {}
        for type in data[app]:
            normalized[app][type] = {}
            for thread in data[app][type]:
                normalized[app][type][thread] = data[app]["LLVM-Polly"]["1"] / data[app][type][thread] if data[app][type][thread] > 0.0 else 0.0

    # draw dashed lines one each of the y-axis ticks
    ymax   = -1000000
    ymin   =  1000000
    yticks = []
    for app in normalized:
        for type in normalized[app]:
            for thread in normalized[app][type]:
                try:
                    if math.log2(normalized[app][type][thread]) > ymax:
                        ymax = math.log2(normalized[app][type][thread])
                    if math.log2(normalized[app][type][thread]) < ymin:
                        ymin = math.log2(normalized[app][type][thread])
                except Exception as e:
                    print("Could not find logarithm of number: "+str(normalized[app][type][thread])+" - "+str(e))
    #for exp in range(math.floor(ymin), math.ceil(ymax)+1):
        #if (exp % 2) == 0:
            #plt.axhline(y=2**exp, color="black", linestyle="dashed")
    i = 0
    for ax in axes:
        app = list(normalized.keys())[i]
        polly  = ax.scatter(THREADS, [normalized[app]["Cyclebite-Template"][x] for x in normalized[app]["Cyclebite-Template"]], label="Cyclebite-Template")
        halide = ax.scatter(THREADS, [normalized[app]["LLVM-Polly"][x] for x in normalized[app]["LLVM-Polly"]], label="LLVM-Polly")
        ax.set_title(app, fontsize=titleFont)
        ax.set_xlabel("Threads", fontsize=axisFont)
        if i == 0:
            ax.set_ylabel("Normalized Execution", fontsize=axisFont)
        ax.set_xscale("log", base=2)
        ax.set_yscale("log", base=2)
        ax.set_ylim([0.5, (2**ymax+ymax)])
        ax.set_aspect("equal")
        i += 1
    fig.legend(frameon=False)

    PrintFigure(plt, args.output)

def plotNormalizedSpeedup(timeMap, args):
	"""
	All times are normalized by the clang -O3 implementation
	"""
	plotOpLevel_scatter(timeMap, args)
	if args.show:
		plt.show()

args = parseArgs()
timeMap = retrieveData(args)
plotNormalizedSpeedup(timeMap, args)
