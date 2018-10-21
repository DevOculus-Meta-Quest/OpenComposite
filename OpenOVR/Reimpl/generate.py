#!/usr/bin/env python3

# List these rather than scanning for files, as this ensures
# that files not checked into version control won't interfere
# with the build process.
interfaces_list = [
    "Compositor",
    "System",
    "Chaperone",
    "Overlay",
    "ChaperoneSetup",
    "RenderModels",
    "Screenshots",
    "Settings",
    "ExtendedDisplay",
    "Applications",
    "Input",
    "ClientCore",
]

import sys, os, glob, re

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../../scripts")

import libparse

context = dict()

libparse.read_context(context, "../OpenVR/interfaces/vrtypes.h", "vr")

bases_header_fn = "static_bases.gen.h"
bases_header = open(bases_header_fn, "w")
bases_header.write("#pragma once\n")
bases_header.write("#include <memory>\n")

base_class_getters = []
def check_base_class_inst(interface, impl, cls):
    if interface in base_class_getters:
        return

    var = "_single_inst_%s" % interface

    base_class_getters.append(interface)

    bases_header.write("class %s;\n" % cls)
    bases_header.write("std::shared_ptr<%s> GetBase%s();\n" % (cls, interface))
    bases_header.write("%s* GetUnsafeBase%s();\n" % (cls, interface))
    bases_header.write("std::shared_ptr<%s> GetCreateBase%s();\n" % (cls, interface))

    impl.write("// Single inst of %s\n" % cls)
    impl.write("static std::weak_ptr<%s> %s;\n" % (cls, var))
    impl.write("static %s *%s_unsafe = NULL;\n" % (cls, var))
    impl.write("std::shared_ptr<%s> GetBase%s() { return %s.lock(); };\n" % (cls, interface, var))
    impl.write("%s* GetUnsafeBase%s() { return %s_unsafe; };\n" % (cls, interface, var))
    impl.write("std::shared_ptr<%s> GetCreateBase%s() {\n" % (cls, interface))
    impl.write("\tstd::shared_ptr<%s> ret = %s.lock();\n" % (cls, var))
    impl.write("\tif(!ret) {\n")
    impl.write("\t\tret = std::shared_ptr<%s>(new %s(), [](%s *obj){ %s_unsafe = NULL; delete obj; });\n" % (cls, cls, cls, var))
    impl.write("\t\t%s = ret;\n" % var)
    impl.write("\t\t%s_unsafe = ret.get();\n" % var)
    impl.write("\t}\n")
    impl.write("\treturn ret;\n")
    impl.write("}\n")

def gen_interface(interface, version, header, impl, basename, namespace="vr", basedir="", base=None):
    if not base:
        base = "Base" + interface

    check_base_class_inst(interface, impl, base)

    iv = "VR%s_%s" % (interface, version)
    namespace = "%s::I%s" % (namespace, iv)
    cname = "C" + iv

    header.write("#include \"%s%s.h\"\n" % (basedir, base))
    header.write("class %s : public %s::IVR%s, public CVRCommon {\n" % (cname, namespace, interface))
    header.write("private:\n")
    header.write("\tconst std::shared_ptr<%s> base;\n" % base)
    header.write("public:\n")
    header.write("\tvirtual void** _GetStatFuncList() override;\n");
    header.write("\t%s();\n" % cname);
    header.write("\t// Interface methods:\n")
    impl.write("// Misc for %s:\n" % cname)
    impl.write("%s::%s() : base(GetCreateBase%s()) {}\n" % (cname, cname, interface))
    impl.write("// Interface methods for %s:\n" % cname)
    filename = "../" + basename
    icontext = dict(context)
    libparse.read_context(icontext, filename, namespace)

    with open(filename) as f:
        funcs = []
        for line in libparse.nice_lines(f):
            func = libparse.parseline(line, icontext)
            if func:
                funcs.append(func)
                args = ", ".join(
                    map(lambda f: "%s %s" % (f.type, f.name), func.args)
                    )

                header.write("\t%s %s(%s);\n" % ( func.return_type, func.name, args ))

                # Generate definition
                # Only do so if the user hasn't defined it
                if not (cname, func.name) in implemented_functions:
                    nargs = []
                    for a in func.args:
                        if namespace in a.type:
                            casttype = a.type.replace(namespace + "::", "OOVR_")
                            nargs.append("(%s) %s" % (casttype, a.name))
                        else:
                            nargs.append(a.name)

                    nargs = ", ".join(nargs)

                    safereturn = "return"
                    if namespace in func.return_type:
                        safereturn += " (%s)" % func.return_type

                    impl.write("%s %s::%s(%s) { %s base->%s(%s); }\n" % ( func.return_type, cname, func.name, args, safereturn, func.name, nargs ))

    header.write("};\n")

    gen_fntable(interface, version, funcs, impl)

def gen_fntable(interface, version, funcs, out):
    cname = "CVR%s_%s" % (interface, version)
    prefix = "fntable_%s_%s" % (interface, version)
    ivarname = "%s_instance" % prefix
    arrvarname = "%s_funcs" % prefix
    fnametemplate = "%s_impl_%%s" % prefix

    out.write("// FnTable for %s:\n" % cname)
    out.write("static %s *%s = NULL;\n" % (cname, ivarname))

    # Generate the stub functions
    for func in funcs:
        nargs = ", ".join([a.name for a in func.args]) # name-only arguments, eg "a, *b, c"
        call_stmt = "return %s->%s(%s);" % (ivarname, func.name, nargs)

        fargs = ", ".join(["%s %s" % (a.type, a.name) for a in func.args]) # full arguments, eg "int a, char *b, uint64_t c"
        out.write("static %s OPENVR_FNTABLE_CALLTYPE %s(%s) { %s }\n" % (func.return_type, fnametemplate % func.name, fargs, call_stmt))

    # Generate the array
    out.write("static void *%s[] = {\n" % arrvarname)
    for func in funcs:
        out.write("\t%s,\n" % fnametemplate % func.name)

    out.write("};\n")

    # Generate the getter
    out.write("void** %s::_GetStatFuncList() { %s = this; return %s; }\n" % (cname, ivarname, arrvarname))

def gen_api_interface(plain_name, version, header, impl, header_name):
    interface = "OC" + plain_name
    funcs = gen_interface(interface, version, header, impl, header_name, namespace="ocapi", basedir="API/", base="OCBase"+plain_name)

geniface = re.compile("GEN_INTERFACE\(\"(?P<interface>\w+)\",\s*\"(?P<version>\d{3})\"(?:\s*,\s*(?P<flags>.*))?\s*\)")
baseflag = re.compile("BASE_FLAG\(\s*(?P<flags>.*)\s*\)")
impldef = re.compile(r"^\w[\w\d\s:]*\s+[\*&]*\s*(?P<cls>[\w\d_]+)::(?P<name>[\w\d_]+)\s*\(.*\)")

impl = open("stubs.gen.cpp", "w")
impl.write("#include \"stdafx.h\"\n")

# The interfaces header contains OPENVR_FNTABLE_CALLTYPE, along with declarations
#  for any non-static functions we define
impl.write("#include \"Interfaces.h\"\n")
impl.write("#include \"%s\"\n" % bases_header_fn)

# Delete the old headers, in case the cpp for one is removed
# Leave the stubs file alone as it'll get overwritten anyway
for filename in glob.glob("GVR*.gen.h"):
    os.remove(filename)

# Keep track of all the interfaces we define
all_interfaces = []

for base_interface in interfaces_list:
    filename = "CVR" + base_interface + ".cpp"
    todo_interfaces = []
    implemented_functions = []

    base_flags = []

    with open(filename) as f:
        for line in libparse.nice_lines(f):
            line = line.strip()
            match = geniface.match(line)
            basematch = baseflag.match(line)
            implmatch = impldef.match(line)
            if match:
                version = match.group("version")
                interface = match.group("interface")
                flags = match.group("flags")
                header_name = "OpenVR/interfaces/IVR%s_%s.h" % (interface, version)
                if flags:
                    flags = [e.strip() for e in flags.split(",")]
                    if "CUSTOM" in flags:
                        header_name = "OpenVR/custom_interfaces/IVR%s_%s.h" % (interface, version)
                    elif "API" in flags:
                        header_name = "API/I%s_%s.h" % (interface, version)
                todo_interfaces.append((interface, version, flags, header_name))
            elif basematch:
                flags = basematch.group("flags")
                base_flags += [e.strip() for e in flags.split(",")]
            elif "GEN_INTERFACE" in line and not "#define" in line and not line.startswith("//"):
                print(line)
                raise RuntimeError("GEN_INTERFACE syntax error!")
            elif implmatch:
                cls = implmatch.group("cls")
                name = implmatch.group("name")
                implemented_functions.append((cls, name))

    if not todo_interfaces:
        continue

    header_filename = "GVR%s.gen.h" % base_interface
    header = open(header_filename, "w")
    header.write("#pragma once\n")
    header.write("#include \"BaseCommon.h\"\n")

    for i in todo_interfaces:
        header.write("#include \"%s\"\n" % i[3])

    impl.write("#include \"%s\"\n" % header_filename)

    for i in todo_interfaces:
        if i[2] and "API" in i[2]:
            gen_api_interface(i[0], i[1], header, impl, i[3])
        else:
            gen_interface(i[0], i[1], header, impl, i[3])

    all_interfaces += todo_interfaces

    header.close()

bases_header.close()

# Generate CreateInterfaceByName
impl.write("// Get interface by name\n")
impl.write("void *CreateInterfaceByName(const char *name) {\n")

for i in all_interfaces:
    interface = i[0]
    ns = "vr"

    if i[2] and "API" in i[2]:
        interface = "OC" + interface
        ns = "ocapi"

    name = "CVR%s_%s" % (interface, i[1])
    var = "%s::IVR%s_%s::IVR%s_Version" % (ns, interface, i[1], interface)

    impl.write("\tif(strcmp(%s, name) == 0) return new %s();\n" % (var, name))

impl.write("\treturn NULL;\n")
impl.write("}\n")

impl.close()
