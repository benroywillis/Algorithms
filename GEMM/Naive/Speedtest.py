import subprocess as sp
import statistics as st
import re
import json

iterations = 15

binary = "GEMM.elf"
pollyBinary = "GEMM.elf_polly"
tikBinary = "ts_GEMM.exec"
tikPollyBinary = "ts_polly_GEMM.exec"

def getExecutionTime(inString):
	"""
	@param[in] log      Absolute path to the logfile to be read
	@retval    time     Time of the execution as a float in seconds
	"""
	time  = -1
	error = []
	try:
		stuff = re.findall("\d+\.\d+",inString)
		if (len(stuff) == 1) and (time < 0):
			numberString = stuff[0].replace("real ","")
			time = float(numberString)
			return time
		else:
			return -1
	except Exception as e:
		print("Error while trying to read execution time: "+str(e))
		return -1

timeMap = dict().fromkeys( [binary, pollyBinary, tikBinary, tikPollyBinary] )
for key in timeMap:
	timeMap[key] = { "Time": { "Times": [], "median": 0.0, "mean": 0.0, "stddev": 0.0 }, \
					 "Dilation": { "Dilations": [], "median": 0.0, "mean": 0.0, "stddev": 0.0 } }

# make all the binaries we will need
build = "make clean ; make elf ; make elf_polly ; make ts_GEMM.exec ; make ts_polly_GEMM.exec"
check = sp.Popen( build, stdout=sp.PIPE, stderr=sp.PIPE, shell=True)
check.wait()

for key in timeMap:
	for i in range( iterations ):
		check = sp.Popen("./"+key, stdout=sp.PIPE, stderr=sp.PIPE, shell=True)
		output = ""
		while check.poll() is None:
			output += check.stdout.read().decode("utf-8")
		timeMap[key]["Time"]["Times"].append( getExecutionTime(output) )
	timeMap[key]["Time"]["median"] = st.median( timeMap[key]["Time"]["Times"] )
	timeMap[key]["Time"]["mean"] = st.mean( timeMap[key]["Time"]["Times"] )
	timeMap[key]["Time"]["stddev"] = st.pstdev( timeMap[key]["Time"]["Times"] )

with open("TimeMap.json", "w") as f:
	json.dump(timeMap, f, indent=4)
