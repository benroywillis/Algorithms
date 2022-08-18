
import argparse
import subprocess as sp
import json
import re

OPFLAGS = ["O0", "O1", "O2", "O3" ]
THREADS = [1, 2, 4, 8, 16]

def parseArgs():
	arg_parser = argparse.ArgumentParser()
	arg_parser.add_argument("-d", "--directory", default="./", help="Specify root directory of project (the directory that contains Naive, Halide, API, etc.")
	arg_parser.add_argument("-o", "--output", default="Speedup", help="Specify output file name.")
	arg_parser.add_argument("-s", "--samples", default=15, help="Specify number of samples taken for each timing experiment.")
	arg_parser.add_argument("-i", "--iterations", default=15, help="Specify number of iterations taken for each sample.")
	arg_parser.add_argument("-mi", "--milliseconds", action="store_true", help="Output timings in milliseconds (default: seconds).")
	arg_parser.add_argument("-sf", "--sig-figs", default=3, help="Output number of significant figures.")
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

args = parseArgs()
timeMap = { "Naive": {}, "Polly": {}, "Halide": {} }

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

