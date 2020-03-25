# About

This tool helps to classify the enterprise state of a target on the network.
It will identify the target as PERSONAL or return the enterprise ID of the corporate resource.

# Usage

Prebuilt binaries are available in the Releases section: https://github.com/microsoft/EnterpriseStateClassify/releases

To use this tool, simply run:

	EnterpriseStateClassify.exe Connect <FLAG> <TARGET> 

# Examples

On the microsoft corporate network,

"EnterpriseStateClassify.exe Connect 3 fb.com" will return:

	The target fb.com
	 Enterprise state: PERSONAL

"EnterpriseStateClassify.exe Connect 3 msw" will return:

	The target: msw
 	 Enterprise state: EnterpriseId=corp.microsoft.com

# Building

Use Visual Studio to build project solution


# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
