import qbs
import "../../buildopts/qthmi_product.qbs" as QTHMILibrary

QTHMILibrary
{
    qthmiModuleName: "QJson"
    type: "dynamiclibrary"
    targetName: "qjson"
    version: "1.0.0"

    files: [ "include/*.h", "src/*.cpp" ]

    cpp.includePaths: base.concat( [ "src" ] )
    cpp.defines: base.concat( [ "QJSONBACKPORT_LIBRARY", "Q_JSONRPC_DLL", "Q_BUILD_JSONRPC" ] )

    extraIncludePaths: [ "./include" ]

    Depends { name: "Qt.core"; link: false }

    Export {
        Depends { name: "cpp"}
        Depends { name: "Qt.core" }

        cpp.includePaths: product.includePaths

    }

    // Default install
    Group {
        fileTagsFilter: [ "dynamiclibrary", "debuginfo_dll", "dynamiclibrary_symlink" ]
        qbs.install: true
        qbs.installDir: "app/"
    }
}
