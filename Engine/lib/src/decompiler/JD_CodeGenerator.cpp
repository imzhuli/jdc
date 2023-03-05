#include <jdc/decompiler/JD_CodeGenerator.hpp>
#include <jdc/decompiler/JD_SessionMarks.hpp>
#include <xel/String.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

using namespace xel;
using namespace std;

namespace jdc
{

    std::string MakeClassIdentifier(const xJavaClass * ClassPtr)
    {
        std::vector<std::string> Qualifiers;
        auto & ClassInfo = ClassPtr->ClassInfo;
        auto & AccessFlags = ClassInfo.AccessFlags;
        if (AccessFlags & ACC_PUBLIC) {
            Qualifiers.push_back("public");
        }
        else if (AccessFlags & ACC_PRIVATE) {
            Qualifiers.push_back("private");
        }
        else if (AccessFlags & ACC_PROTECTED) {
            Qualifiers.push_back("protected");
        }

        if (AccessFlags & ACC_STATIC) {
            Qualifiers.push_back("static");
        }


        if (AccessFlags & ACC_ENUM) {
            Qualifiers.push_back("enum");
            Qualifiers.push_back(ClassPtr->InnermostCodeName);
        } else {
            if (AccessFlags & ACC_ABSTRACT) {
                Qualifiers.push_back("abstract");
            }
            if (AccessFlags & ACC_FINAL) {
                Qualifiers.push_back("final");
            }
            Qualifiers.push_back("class");
            Qualifiers.push_back(ClassPtr->InnermostCodeName);
            Qualifiers.push_back("extends");
            Qualifiers.push_back(ConvertBinaryNameToCodeName(ClassInfo.GetConstantClassBinaryName(ClassInfo.SuperClass)));
        }

        if (ClassInfo.InterfaceIndices.size()) {
            Qualifiers.push_back("\nimplements\n");
            std::vector<std::string> Interfaces;
            for (auto Index : ClassInfo.InterfaceIndices) {
                Interfaces.push_back(ConvertBinaryNameToCodeName(ClassInfo.GetConstantClassBinaryName(Index)));
            }
            Qualifiers.push_back(JoinStr(Interfaces, ",\n "));
        }

        return xel::JoinStr(Qualifiers, ' ');
    }

    void BuildPakcageClassSource(std::ofstream & Output, const std::string & PackageName)
    {
        Output << SM_PACKAGE << endl;
        Output << "package " << PackageName << ";" << endl;
        Output << endl;
    }

    bool BuildOuterClassSource(std::ofstream & Output, const xJavaClass * ClassPtr)
    {
        Output << SM_CLASS_IDENTIFIER_ << ClassPtr->BinaryName << endl;
        Output << MakeClassIdentifier(ClassPtr) << endl;

        Output << "{" << endl;
        Output << SM_CLASS_BODY_BEGIN_ << ClassPtr->BinaryName << endl;
        Output << endl;

        Output << SM_CLASS_BODY_END_ << ClassPtr->BinaryName << endl;
        Output << "}" << endl;
        Output << endl;

        return true;
    }

    std::string BuildInnerClassSource(const xJavaClass * ClassPtr)
    {
        std::ostringstream Output;

        Output << SM_CLASS_IDENTIFIER_ << ClassPtr->BinaryName << endl;
        Output << MakeClassIdentifier(ClassPtr) << endl;

        Output << "{" << endl;
        Output << SM_CLASS_BODY_BEGIN_ << ClassPtr->BinaryName << endl;
        Output << endl;

        Output << SM_CLASS_BODY_END_ << ClassPtr->BinaryName << endl;
        Output << "}" << endl;
        Output << endl;

        return Output.str();
    }


    bool ResetClassSource(const std::filesystem::path & RootDir, const xJavaSpace * JavaSpacePtr, const xJavaClass * ClassPtr)
    {
        auto Path = RootDir / ClassPtr->PackageBinaryName;
        std::error_code Error;
        auto Filename = Path / ClassPtr->Extend.SourceFilename;
        std::filesystem::create_directories(Path, Error);
        if (Error) {
            X_DEBUG_PRINTF("BuildClassSource error: %s\n", Error.message().c_str());
            return false;
        }
        auto Output = std::ofstream(Filename, std::ios_base::binary | std::ios_base::trunc);
        BuildPakcageClassSource(Output, ClassPtr->PackageCodeName);
        return true;
    }

    bool BuildClassSource(const std::filesystem::path & RootDir, const xJavaSpace * JavaSpacePtr,  const xJavaClass * ClassPtr)
    {
        auto Path = RootDir / ClassPtr->PackageBinaryName;
        auto Filename = Path / ClassPtr->Extend.SourceFilename;
        if (!ClassPtr->IsInnerClass()) {
            auto Output = std::ofstream(Filename, std::ios_base::binary | std::ios_base::app);
            if (!Output) {
                X_DEBUG_PRINTF("BuildClassSource error: failed to create file: %s\n", Filename.c_str());
                return false;
            }
            return BuildOuterClassSource(Output, ClassPtr);
        }
        else {
            auto Input = FileToStr(Filename);
            if (!Input()) {
                X_DEBUG_PRINTF("BuildClassSource error: failed to open file: %s\n", Filename.c_str());
                return false;
            }
            auto Original = *Input;

            // body end string:
            auto & ClassInfo = ClassPtr->ClassInfo;
            auto & ParentClassBinaryName = ClassInfo.GetOuterClassBinaryName();

            std::string ParentBodyEndString = SM_CLASS_BODY_END_ + ParentClassBinaryName + '\n';
            auto InsertIndex = Original.find(ParentBodyEndString);
            assert(InsertIndex != Original.npos);

            auto Output = std::ofstream(Filename, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
            if (!Output) {
                X_DEBUG_PRINTF("BuildClassSource error: failed to create file: %s\n", Filename.c_str());
                return false;
            }

            Output << std::string_view(Original.data(), InsertIndex);
            Output << BuildInnerClassSource(ClassPtr);
            Output << std::string_view(Original.data() + InsertIndex, Original.size() - InsertIndex);
        }

        return true;
    }

}
