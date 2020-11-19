#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
import os.path
import shutil
import sys
import re
import subprocess

try:
    import configparser
except ImportError:
    import ConfigParser as configparser

def createConfig(pathConfig):
    #path for project binary file
    srcBinDir = os.path.join(os.path.dirname(pathConfig), "/release", )
    srcBinDir = os.path.normpath(srcBinDir)
    srcBinPath = os.path.join(srcBinDir, "SqlViewGenerator.exe")
    
    nameInstaller = "SqlViewGeneratorInstaller.exe"
    
    qtBinaryDir = os.path.join("C:", "\Dev", "Qt", "5.15.1", "mingw81_64", "bin")
    while not os.path.exists(qtBinaryDir): 
        qtBinaryDir = input("Path to Qt bin: ")
        
    binarycreator = os.path.join(qtBinaryDir, "..", "..", "..", "Tools", "QtInstallerFramework", "3.2", "bin", "binarycreator.exe")
    while not os.path.exists(binarycreator):
        binarycreator = input("Path to binatycreator.exe: ")

    postgresBinPath = os.path.join("C:", "\Program Files", "\MySQL\MySQL Server 8.0\lib")
    while not os.path.exists(postgresBinPath):
        postgresBinPath = input("Path to PostgresSQL bin dir: ")

    config = configparser.ConfigParser()
    config.add_section("Settings")

    config.set("Settings", "src_binary_path", srcBinPath)
    config.set("Settings", "name_installer", nameInstaller)
    config.set("Settings", "binarycreator_path", binarycreator)
    config.set("Settings", "qt_binary_dir", qtBinaryDir)
    config.set("Settings", "postgresql_bin_dir", postgresBinPath)
    
    with open(pathConfig, "w") as config_file:
        config.write(config_file)


def findFiles(dirPath, fileList):
    pathList = []
    for element in os.scandir(dirPath):
        if element.is_file():
            if element.name in fileList:
                if element.path.find("Release") >= 0:
                    pathList.append(element.path)
                    fileList.remove(element.name)
                    if fileList.count == 0:
                        break
        else:
            pathList.extend(findFiles(element.path, fileList))
    return pathList


if sys.platform == "linux" or sys.platform == "linux2":
    print("Not implemented for linux")
    exit(0)


ScriptPath = os.path.abspath(__file__)
ScriptDir = os.path.dirname(ScriptPath)

ProjectPath = os.path.join(ScriptPath, "../", "../", "../")
ProjectPath = os.path.normpath(ProjectPath)

settingsFilePath = os.path.join(ScriptDir, "settings.ini")
if not os.path.exists(settingsFilePath):
    createConfig(settingsFilePath)
    print("Created a configuration file, fill in the data",  settingsFilePath)
    exit(0)

config = configparser.ConfigParser()
config.read(settingsFilePath)

SrcBinPath = config.get("Settings", "src_binary_path")
if not os.path.exists(SrcBinPath) or not os.path.isfile(SrcBinPath):
    print("Not found src binary file - ", SrcBinPath)
    exit(-1)

DstBinDir = os.path.join(ScriptDir, "packages", "ru.ssl.passport", "data")
if os.path.exists(DstBinDir):
    shutil.rmtree(DstBinDir)

os.makedirs(DstBinDir)

print("Copy binary file...")
shutil.copy(SrcBinPath, DstBinDir)

qtBinatyDir = config.get("Settings", "qt_binary_dir")



qtenv2 = os.path.join(qtBinatyDir, "qtenv2.bat")

deployDllCmd = []

print("Copy dependency qpdf.dll")
QPdfDllPath = findFiles(ProjectPath, ["qpdf.dll"])
print("qpdf lib: ", QPdfDllPath)
for path in QPdfDllPath:
    if not os.path.exists(path):
        print("not fount - ", path)
        exit(-1)
    shutil.copy(path, DstBinDir)
    deployDllCmd = [qtenv2, "&&", "windeployqt", path, "-dir", DstBinDir]
    if os.system(" ".join(deployDllCmd)) != 0:
        print("Error windeployqt.exe")
        exit(-1)


print("Copy Qt dependency...")
dstBinPath = os.path.join(DstBinDir, os.path.basename(SrcBinPath))
cmd = [qtenv2, "&&", "windeployqt.exe", dstBinPath]
if os.system(" ".join(cmd)) != 0:
    print("Error windeployqt.exe")
    exit(-1)


print("Copy dependency postgresql driver...")
PostgreSqlDir = config.get("Settings", "postgresql_bin_dir")
if not os.path.exists(PostgreSqlDir):
    print("Not fount libs postgresql")
    exit(-1)

#or copy all dll files
#libs = ["libpq.dll", "libssl-1_1.dll", "libcrypto-1_1.dll", "libintl-8.dll", "libiconv-2.dll"]
libs = ["libmysql.dll"]
for lib in libs:
    path = os.path.join(PostgreSqlDir, lib)
    if not os.path.exists(path):
        print("not fount - ", path)
        exit(-1)
    shutil.copy(path, DstBinDir)


ConfigFilePath = os.path.join(ScriptDir, "config", "config.xml")
PackagesDir = os.path.join(ScriptDir, "packages")

BinaryCreatorPath = config.get("Settings", "binarycreator_path")
if not os.path.exists(BinaryCreatorPath) :
    print("Not found ", BinaryCreatorPath)
    exit(-1)

NameInstaller = config.get("Settings", "name_installer")

create_installer_cmd = [BinaryCreatorPath, "--offline-only", "-c", ConfigFilePath, "-p", PackagesDir, NameInstaller]
create_installer_cmd = " ".join(create_installer_cmd)

print("Create installer...")
if 0 == os.system(create_installer_cmd):
    print("Completed")
else:
   print("Error")
