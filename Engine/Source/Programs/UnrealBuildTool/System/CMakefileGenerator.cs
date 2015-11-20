﻿// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace UnrealBuildTool
{
	/// <summary>
	/// Represents a folder within the master project (e.g. Visual Studio solution)
	/// </summary>
	public class CMakefileFolder : MasterProjectFolder
	{
		/// <summary>
		/// Constructor
		/// </summary>
		public CMakefileFolder(ProjectFileGenerator InitOwnerProjectFileGenerator, string InitFolderName)
			: base(InitOwnerProjectFileGenerator, InitFolderName)
		{
		}
	}

	public class CMakefileProjectFile : ProjectFile
	{
		public CMakefileProjectFile(FileReference InitFilePath)
			: base(InitFilePath)
		{
		}
	}
	/// <summary>
	/// CMakefile project file generator implementation
	/// </summary>
	public class CMakefileGenerator : ProjectFileGenerator
	{
		/// True if intellisense data should be generated (takes a while longer)
		bool bGenerateIntelliSenseData = false;

		/// Default constructor
		public CMakefileGenerator(FileReference InOnlyGameProject)
			: base(InOnlyGameProject)
		{
		}

		/// True if we should include IntelliSense data in the generated project files when possible
		override public bool ShouldGenerateIntelliSenseData()
		{
			return bGenerateIntelliSenseData;
		}

		/// File extension for project files we'll be generating (e.g. ".vcxproj")
		override public string ProjectFileExtension
		{
			get
			{
				return ".txt";
			}
		}

		protected override bool WriteMasterProjectFile(ProjectFile UBTProject)
		{
			bool bSuccess = true;
			return bSuccess;
		}

		private bool WriteCMakeLists()
		{
			string BuildCommand = "";
			var FileName = "CMakeLists.txt";
			var CMakefileContent = new StringBuilder();

			var CMakeSectionEnd = " )\n\n";

			var CMakeSourceFilesList = "set(SOURCE_FILES \n";
			var CMakeHeaderFilesList = "set(HEADER_FILES \n";
			var CMakeConfigFilesList = "set(CONFIG_FILES \n";
			var IncludeDirectoriesList = "include_directories( \n";
			var PreprocessorDefinitionsList = "add_definitions( \n";

			var CMakeGameRootPath = "";
			var CMakeUE4RootPath = "set(UE4_ROOT_PATH " + Path.GetFullPath(ProjectFileGenerator.RootRelativePath) + ")\n";

			string GameProjectPath = "";
			string GameProjectFile = "";

			string CMakeGameProjectFile = "";

			bool bIsMac = BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Mac;
			bool bIsLinux = BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Linux;
			bool bIsWin64 = BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win64;

			String HostArchitecture = null;
			if (bIsLinux)
			{
				HostArchitecture = "Linux";
			}
			else if (bIsMac)
			{
				HostArchitecture = "Mac";
			}
			else if (bIsWin64)
			{
				HostArchitecture = "Win64";
			}
			else
			{
				throw new BuildException("ERROR: CMakefileGenerator does not support this platform");
			}

			if (!String.IsNullOrEmpty(GameProjectName))
			{
				CMakeGameRootPath = "set(GAME_ROOT_PATH \"" + OnlyGameProject.Directory.FullName + "\")\n";

				GameProjectPath = OnlyGameProject.Directory.FullName;
				GameProjectFile = OnlyGameProject.FullName;

				CMakeGameProjectFile = "set(GAME_PROJECT_FILE \"" + GameProjectFile + "\")\n";

				BuildCommand = "set(BUILD mono ${UE4_ROOT_PATH}/Engine/Binaries/DotNET/UnrealBuildTool.exe )\n";
			}
			else if (bIsLinux || bIsMac)
			{
				BuildCommand = String.Format("set(BUILD cd ${{UE4_ROOT_PATH}} && bash ${{UE4_ROOT_PATH}}/Engine/Build/BatchFiles/{0}/Build.sh)\n", HostArchitecture);
			}
			else if (bIsWin64)
			{
				BuildCommand = "set(BUILD bash ${{UE4_ROOT_PATH}}/Engine/Build/BatchFiles/Build.bat)\n";
			}

			CMakefileContent.Append(
				"# Makefile generated by CMakefileGenerator.cs\n" +
				"# *DO NOT EDIT*\n\n" +
				"cmake_minimum_required (VERSION 2.6)\n" +
				"project (UE4)\n\n" +
				CMakeUE4RootPath +
				CMakeGameProjectFile +
				BuildCommand +
				CMakeGameRootPath + "\n"
			);

			List<String> IncludeDirectories = new List<String>();
			List<String> PreprocessorDefinitions = new List<String>();

			foreach (var CurProject in GeneratedProjectFiles)
			{
				foreach (var CurPath in CurProject.IntelliSenseIncludeSearchPaths)
				{
					string IncludeDirectory = GetIncludeDirectory(CurPath, Path.GetDirectoryName(CurProject.ProjectFilePath.FullName));
					if (IncludeDirectory != null && !IncludeDirectories.Contains(IncludeDirectory))
					{
						IncludeDirectories.Add(IncludeDirectory);
					}
				}

				foreach (var CurDefinition in CurProject.IntelliSensePreprocessorDefinitions)
				{
					string Definition = CurDefinition;
					string AlternateDefinition = Definition.Contains("=0") ? Definition.Replace("=0", "=1") : Definition.Replace("=1", "=0");
					if (Definition.Equals("WITH_EDITORONLY_DATA=0") || Definition.Equals("WITH_DATABASE_SUPPORT=1"))
					{
						Definition = AlternateDefinition;
					}
					if (!PreprocessorDefinitions.Contains(Definition) && !PreprocessorDefinitions.Contains(AlternateDefinition) && !Definition.StartsWith("UE_ENGINE_DIRECTORY") && !Definition.StartsWith("ORIGINAL_FILE_NAME"))
					{
						PreprocessorDefinitions.Add(Definition);
					}
				}
			}

			// Create SourceFiles, HeaderFiles, and ConfigFiles sections.
			var AllModuleFiles = DiscoverModules(FindGameProjects());
			foreach (FileReference CurModuleFile in AllModuleFiles)
			{
				var FoundFiles = SourceFileSearch.FindModuleSourceFiles(CurModuleFile);
				foreach (FileReference CurSourceFile in FoundFiles)
				{

					string SourceFileRelativeToRoot = CurSourceFile.MakeRelativeTo(UnrealBuildTool.EngineDirectory);
					// Exclude files/folders on a per-platform basis.
					if ((bIsLinux && IsLinuxFiltered(SourceFileRelativeToRoot)) || (bIsMac && IsMacFiltered(SourceFileRelativeToRoot))
						|| (bIsWin64 && IsWinFiltered(SourceFileRelativeToRoot))
					)
					{
						if (SourceFileRelativeToRoot.EndsWith(".cpp"))
						{
							if (!SourceFileRelativeToRoot.StartsWith("..") && !Path.IsPathRooted(SourceFileRelativeToRoot))
							{
								// SourceFileRelativeToRoot = "Engine/" + SourceFileRelativeToRoot;
								CMakeSourceFilesList += ("\t\"${UE4_ROOT_PATH}/Engine/" + SourceFileRelativeToRoot + "\"\n");
							}
							else
							{
								if (String.IsNullOrEmpty(GameProjectName))
								{
									// SourceFileRelativeToRoot = SourceFileRelativeToRoot.Substring (3);
									CMakeSourceFilesList += ("\t\"" + SourceFileRelativeToRoot.Substring(3) + "\"\n");
								}
								else
								{
									CMakeSourceFilesList += ("\t\"${GAME_ROOT_PATH}/" + CurSourceFile.MakeRelativeTo(new DirectoryReference(Path.GetDirectoryName(GameProjectPath))) + "\"\n");
								}
							}
						}
						if (SourceFileRelativeToRoot.EndsWith(".h"))
						{
							if (!SourceFileRelativeToRoot.StartsWith("..") && !Path.IsPathRooted(SourceFileRelativeToRoot))
							{
								// SourceFileRelativeToRoot = "Engine/" + SourceFileRelativeToRoot;
								CMakeHeaderFilesList += ("\t\"${UE4_ROOT_PATH}/Engine/" + SourceFileRelativeToRoot + "\"\n");
							}
							else
							{
								if (String.IsNullOrEmpty(GameProjectName))
								{
									// SourceFileRelativeToRoot = SourceFileRelativeToRoot.Substring (3);
									CMakeHeaderFilesList += ("\t\"" + SourceFileRelativeToRoot.Substring(3) + "\"\n");
								}
								else
								{
									CMakeHeaderFilesList += ("\t\"${GAME_ROOT_PATH}/" + Utils.MakePathRelativeTo(CurSourceFile.FullName, GameProjectPath) + "\"\n");
								}
							}
						}
						if (SourceFileRelativeToRoot.EndsWith(".cs"))
						{
							if (!SourceFileRelativeToRoot.StartsWith("..") && !Path.IsPathRooted(SourceFileRelativeToRoot))
							{
								// SourceFileRelativeToRoot = "Engine/" + SourceFileRelativeToRoot;
								CMakeConfigFilesList += ("\t\"${UE4_ROOT_PATH}/Engine/" + SourceFileRelativeToRoot + "\"\n");

							}
							else
							{
								if (String.IsNullOrEmpty(GameProjectName))
								{
									// SourceFileRelativeToRoot = SourceFileRelativeToRoot.Substring (3);
									CMakeConfigFilesList += ("\t\"" + SourceFileRelativeToRoot.Substring(3) + "\"\n");
								}
								else
								{
									CMakeConfigFilesList += ("\t\"${GAME_ROOT_PATH}/" + Utils.MakePathRelativeTo(CurSourceFile.FullName, GameProjectPath) + "\"\n");
								};
							}
						}
					}
				}

			}

			foreach (string IncludeDirectory in IncludeDirectories)
			{
				IncludeDirectoriesList += ("\t\"" + IncludeDirectory + "\"\n");
			}

			foreach (string PreprocessorDefinition in PreprocessorDefinitions)
			{
				PreprocessorDefinitionsList += ("\t-D" + PreprocessorDefinition + "\n");
			}

			// Add section end to section strings;
			CMakeSourceFilesList += CMakeSectionEnd;
			CMakeHeaderFilesList += CMakeSectionEnd;
			CMakeConfigFilesList += CMakeSectionEnd;
			IncludeDirectoriesList += CMakeSectionEnd;
			PreprocessorDefinitionsList += CMakeSectionEnd;

			// Append sections to the CMakeLists.txt file
			CMakefileContent.Append(CMakeSourceFilesList);
			CMakefileContent.Append(CMakeHeaderFilesList);
			CMakefileContent.Append(CMakeConfigFilesList);
			CMakefileContent.Append(IncludeDirectoriesList);
			CMakefileContent.Append(PreprocessorDefinitionsList);

			string CMakeProjectCmdArg = "";

			foreach (var Project in GeneratedProjectFiles)
			{
				foreach (var TargetFile in Project.ProjectTargets)
				{
					if (TargetFile.TargetFilePath == null)
					{
						continue;
					}

					var TargetName = TargetFile.TargetFilePath.GetFileNameWithoutAnyExtensions();		// Remove both ".cs" and ".

					foreach (UnrealTargetConfiguration CurConfiguration in Enum.GetValues(typeof(UnrealTargetConfiguration)))
					{
						if (CurConfiguration != UnrealTargetConfiguration.Unknown && CurConfiguration != UnrealTargetConfiguration.Development)
						{
							if (UnrealBuildTool.IsValidConfiguration(CurConfiguration))
							{
								if (TargetName == GameProjectName || TargetName == (GameProjectName + "Editor"))
								{
									CMakeProjectCmdArg = " -project=\"\\\"${GAME_PROJECT_FILE}\\\"\"";
								}
								var ConfName = Enum.GetName(typeof(UnrealTargetConfiguration), CurConfiguration);
								CMakefileContent.Append(String.Format("add_custom_target({0}-{3}-{1} ${{BUILD}} {2} {0} {3} {1} $(ARGS))\n", TargetName, ConfName, CMakeProjectCmdArg, HostArchitecture));
							}
						}
					}

					if (TargetName == GameProjectName || TargetName == (GameProjectName + "Editor"))
					{
						CMakeProjectCmdArg = " -project=\"\\\"${GAME_PROJECT_FILE}\\\"\"";
					}
					if (HostArchitecture != null)
					{
						CMakefileContent.Append(String.Format("add_custom_target({0} ${{BUILD}} {1} {0} {2} Development $(ARGS) SOURCES ${{SOURCE_FILES}} ${{HEADER_FILES}} ${{CONFIG_FILES}})\n\n", TargetName, CMakeProjectCmdArg, HostArchitecture));
					}
				}
			}

			var FullFileName = Path.Combine(MasterProjectPath.FullName, FileName);
			return WriteFileIfChanged(FullFileName, CMakefileContent.ToString());
		}

		private bool IsLinuxFiltered(String SourceFileRelativeToRoot)
		{
			// minimal filtering as it is helpful to be able to look up symbols from other platforms
			return !SourceFileRelativeToRoot.Contains("Source/ThirdParty/");
		}

		private bool IsMacFiltered(String SourceFileRelativeToRoot)
		{
			return !SourceFileRelativeToRoot.Contains("Source/ThirdParty/") &&
				!SourceFileRelativeToRoot.Contains("/Windows/") &&
				!SourceFileRelativeToRoot.Contains("/Linux/") &&
				!SourceFileRelativeToRoot.Contains("/VisualStudioSourceCodeAccess/") &&
				!SourceFileRelativeToRoot.Contains("/WmfMedia/") &&
				!SourceFileRelativeToRoot.Contains("/WindowsDeviceProfileSelector/") &&
				!SourceFileRelativeToRoot.Contains("/WindowsMoviePlayer/") &&
				!SourceFileRelativeToRoot.Contains("/WinRT/");
		}

		private bool IsWinFiltered(String SourceFileRelativeToRoot)
		{
			return false;
		}

		/// Adds the include directory to the list, after converting it to relative to UE4 root
		private string GetIncludeDirectory(string IncludeDir, string ProjectDir)
		{
			string FullProjectPath = Path.GetFullPath(ProjectFileGenerator.MasterProjectPath.FullName);
			string FullPath = "";
			if (IncludeDir.StartsWith("/") && !IncludeDir.StartsWith(FullProjectPath))
			{
				// Full path to a fulder outside of project
				FullPath = IncludeDir;
			}
			else
			{
				FullPath = Path.GetFullPath(Path.Combine(ProjectDir, IncludeDir));
				FullPath = Utils.MakePathRelativeTo(FullPath, FullProjectPath);
				FullPath = FullPath.TrimEnd('/');
			}
			return FullPath;
		}

		/// ProjectFileGenerator interface
		//protected override bool WriteMasterProjectFile( ProjectFile UBTProject )
		protected override bool WriteProjectFiles()
		{
			return WriteCMakeLists();
		}

		/// ProjectFileGenerator interface
		public override MasterProjectFolder AllocateMasterProjectFolder(ProjectFileGenerator InitOwnerProjectFileGenerator, string InitFolderName)
		{
			return new CMakefileFolder(InitOwnerProjectFileGenerator, InitFolderName);
		}

		/// ProjectFileGenerator interface
		/// <summary>
		/// Allocates a generator-specific project file object
		/// </summary>
		/// <param name="InitFilePath">Path to the project file</param>
		/// <returns>The newly allocated project file object</returns>
		protected override ProjectFile AllocateProjectFile(FileReference InitFilePath)
		{
			return new CMakefileProjectFile(InitFilePath);
		}

		/// ProjectFileGenerator interface
		public override void CleanProjectFiles(DirectoryReference InMasterProjectDirectory, string InMasterProjectName, DirectoryReference InIntermediateProjectFilesDirectory)
		{
		}
	}
}
