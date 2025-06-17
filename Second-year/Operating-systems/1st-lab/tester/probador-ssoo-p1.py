'''
Sistemas Operativos - Practica 1 - Llamadas al sistema

Este programa verifica que el formato del entregable de la practica es el correcto (sigue las especificaciones de nombrado, y esta bien comprimido).

'''
import subprocess
import signal
import os
import glob
import time
import sys
import stat
import filecmp

resultString =""

def normalizeOutput(result):
	'''
	Funcion que normaliza una cadena de entrada con el fin de eliminar errores de formato. Reemplaza saltos de linea por tabuladores y elimina espacios repetidos y espacios al principio y final
	'''
	result = result.replace("\r\n","\n");
	result = result.replace("\r","\n");
	resultList = result.split("\n")
	resultNormalized = ""
	for resultLine in resultList:
		if resultLine == "":
			continue
		resultLine = resultLine.replace("\t", " ")
		resultLine = resultLine.replace("  ", " ")
		resultLine = resultLine.strip()
		resultNormalized += (resultLine + "\t")

	resultNormalized = resultNormalized.strip()

	return resultNormalized


def lookForTargetFile(folder,targetFile):
	'''
	Funcion que busca un fichero objetivo en una carpeta y devuelve su ruta
	'''
	toReturn="-"
	found=False
	subfolders=[x[0] for x in os.walk(folder)]
	for innerFolder in subfolders:
		folders=os.listdir(innerFolder)
		for item in folders:
			if item.lower()==targetFile.lower():
				return innerFolder+"/"+item
	return toReturn


def get_permissions_octal(filepath):
	mode = os.stat(filepath).st_mode  # Obtiene los permisos del archivo
	return oct(mode & 0o777)  # Extrae solo los bits de permiso y los convierte a octal


def checkTargetFile(folder,targetFile):
	'''
	Funcion que comprueba que un fichero objetivo existe. Si no existe, lo intenta buscar en las carpetas hijas.
	'''
	toReturn="-"
	filePath=folder+targetFile
	if not os.path.exists(filePath):
		toReturn=lookForTargetFile(folder,targetFile)
	else:
		toReturn=filePath
	return toReturn


def testCrear(folder):
	'''
	Funcion que realiza la prueba sugerida del programa mycat
	'''

	global resultString 
	#Definimos los nombres del fichero de codigo fuente, programa y fichero de prueba
	targetFile="crear.c"
	programFile="crear"
	
	#Buscamos el fichero fuente
	filePath=checkTargetFile(folder,targetFile)
	if filePath=="-":
		print("CORRECTOR:", targetFile, "no encontrado")
		return "-"
	
	#Compilamos el fichero fuente
	programPath=folder+programFile
	compiled=subprocess.call(["gcc", filePath,"-o", programPath])
	if compiled != 0:
		print("CORRECTOR:", "error al compilar", targetFile)
		return "-"


	#Realizamos prueba 1
	result1 = subprocess.check_output(["cat", folder+testFile])

	#Ejecutamos el programa del alumno
	cmd = programPath + " " + folder+testFile
	pro = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, preexec_fn=os.setsid)  
	time.sleep(0.5)
	os.killpg(pro.pid, signal.SIGTERM) 

	result2, stderr = pro.communicate()

	#Obtenemos las salidas de la prueba con la que vamos a comparar y de la ejecucion del programa del alumno
	expectedResult = normalizeOutput(result1)
	obtainedResult = normalizeOutput(result2)

	#Comparamos las salidas e imprimimos resultados
	print("CORRECTOR MYCAT. Salida esperada:" )
	print(result1)
	print("CORRECTOR MYCAT. Salida del programa:")
	print(result2)

	if result1 == result2:
		print("CORRECTOR MYCAT. PRUEBA CORRECTA")
		resultString += "1 " 
	else:
		print("CORRECTOR MYCAT. PRUEBA INCORRECTA. Las salidas difieren")
		resultString += "0 "

	print("")
	print("")
	print("*********************************************************")
	print("")
	print("")

	#Realizamos la prueba sugerida (cat <fichero_big>)
	result1 = subprocess.check_output(["cat", folder+"f_big.txt"])

	#Ejecutamos el programa del alumno
	cmd = programPath + " " + folder+"f_big.txt"
	pro = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, preexec_fn=os.setsid)
	time.sleep(0.5)
	os.killpg(pro.pid, signal.SIGTERM)

	result2, stderr = pro.communicate()

	#Obtenemos las salidas de la prueba con la que vamos a comparar y de la ejecucion del programa del alumno
	expectedResult = normalizeOutput(result1)
	obtainedResult = normalizeOutput(result2)

	#Comparamos las salidas e imprimimos resultados
	print("CORRECTOR MYCAT. Salida esperada:")
	print(result1)
	print("CORRECTOR MYCAT. Salida del programa:")
	print(result2)

	if result1 == result2:
		print("CORRECTOR MYCAT. PRUEBA CORRECTA")
		resultString += "1 "
	else:
		print("CORRECTOR MYCAT. PRUEBA INCORRECTA. Las salidas difieren")
		resultString += "0 "

	print("")
	print("")
	print("*********************************************************")
	print("")
	print("")




if(__name__=="__main__"):
	'''
	Funcion main de la aplicacion. Obtiene el fichero pasado como argumento. Lo descomprime, chequea su formato y finalmente lanza las pruebas.
	'''

	#Comprobamos que se ha pasado un fichero como argumento
	if not len(sys.argv) == 2:
		print('Uso: python corrector_ssoo.py <fichero zip>')
	else:
		print('CORRECTOR: corrigiendo', sys.argv[1])
		inputFile = sys.argv[1]
		
		#Comprobamos que el fichero existe
		if not os.path.isfile(inputFile):
			print("El fichero", inputFile, "no existe")
			sys.exit(0)
	
		#Comprobamos el formato del nombre del fichero
		tokens=inputFile.replace(".zip","")
		tokens=tokens.split("_")
		if len(tokens) != 3 and len(tokens) != 4 and len(tokens) != 5:
			print("Formato del nombre del archivo incorrecto: ssoo_p1_AAAAA_BBBBB_CCCCC.zip")
			sys.exit(0)
			
		ssoo=tokens[0]
		p1=tokens[1]
		u1=tokens[2]
		u2=""
		u3=""
		if len(tokens)>3:
			u2=tokens[3]
		if len(tokens)>4:
			u3=tokens[4]
		if not (ssoo == "ssoo" and p1 == "p1"):
			print("Formato del nombre del archivo incorrecto: ssoo_p1_AAAAA_BBBBB_CCCCC.zip")
			sys.exit(0)

		print("CORRECTOR: NIA 1",u1, "NIA 2", u2, "NIA 3", u3)

		print('CORRECTOR: descomprimiendo')

		#Descomprimimos el fichero en la carpeta temporal
		tempFolder="./"
		zipresult=subprocess.call(["unzip",tempFolder+"/*.zip","-d",tempFolder])
		if not zipresult == 0:
			print("Error al descomprimir el fichero zip")
			sys.exit(0)

		# Compilar
		makeres=subprocess.call(["make"])
		if not makeres == 0:
			print("Error de compilación")
			sys.exit(0)
		else:
			print("Compilación correcta")

		#Realizamos unas pruebas basicas de cada uno de los programas
		cmd = "./crear test.txt 0777"
		pro = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, preexec_fn=os.setsid)
		time.sleep(0.5)
		result, stderr = pro.communicate()
		res=get_permissions_octal("test.txt")
		#print(res)
		if res == "0o777":
			print("Test 1: OK")
			resultString += "1 "
		else:
			print("Test 1: Error")
			resultString += "0 "

		cmd = "./crear test2.txt 0000"
		pro = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, preexec_fn=os.setsid)
		time.sleep(0.5)
		result, stderr = pro.communicate()
		res=get_permissions_octal("test2.txt")
		#print(res)
		if res == "0o0":
			print("Test 2: OK")
			resultString += "1 "
		else:
			print("Test 2: Error")
			resultString += "0 "

		cmd = "./crear test3.txt 0644"
		pro = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, preexec_fn=os.setsid)
		time.sleep(0.5)
		result, stderr = pro.communicate()
		res=get_permissions_octal("test3.txt")
		#print(res)
		if res == "0o644":
			print("Test 3: OK")
			resultString += "1 "
		else:
			print("Test 3: Error")
			resultString += "0 "

		cmd = "./combine"
		pro = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, preexec_fn=os.setsid)
		time.sleep(0.5)
		result, stderr = pro.communicate()
		if result != 0:
			print("Test 4: OK")
			resultString += "1 "
		else:
			print("Test 4: Error")
			resultString += "0 "

		cmd = "./combine f1.dat f2.dat test5.txt"
		pro = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, preexec_fn=os.setsid)
		time.sleep(0.5)

		if filecmp.cmp("test5.txt", "combine1res", shallow=False):
			print("Test 5-a: OK")
			resultString += "1 "
		else:
			print("Test 5-a: Error")
			resultString += "0 "

		if filecmp.cmp("estadisticas.csv", "combine1stats", shallow=False):
			print("Test 5-b: OK")
			resultString += "1 "
		else:
			print("Test 5-b: Error")
			resultString += "0 "

		cmd = "./combine f3.dat f4.dat test6.txt"
		pro = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, preexec_fn=os.setsid)
		time.sleep(0.5)

		if filecmp.cmp("test6.txt", "combine2res", shallow=False):
			print("Test 6-a: OK")
			resultString += "1 "
		else:
			print("Test 6-a: Error")
			resultString += "0 "

		if filecmp.cmp("estadisticas.csv", "combine2stats", shallow=False):
			print("Test 6-b: OK")
			resultString += "1 "
		else:
			print("Test 6-b: Error")
			resultString += "0 "

		print(resultString)

		#subprocess.call(["rm","-r","./tmp"])









