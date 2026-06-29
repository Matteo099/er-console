import re
from pathlib import Path


# =============================================================================
# Paths
# =============================================================================

PROJECT_DIR = Path(__file__).parent.parent

BINDINGS_DIR = (
    PROJECT_DIR
    / "src"
    / "systems"
    / "lua"
    / "bindings"
)

OUTPUT_FILE = (
    PROJECT_DIR
    / "lua_stubs"
    / "er_api.lua"
)


# =============================================================================
# Regex
# =============================================================================

LUA_PATTERN = re.compile(
    r"//\s*@lua\s+"
    r"([a-zA-Z0-9_.]+)"
    r"\((.*?)\)"
    r"\s*(?:->\s*([a-zA-Z0-9_]+))?"
)

DOC_PATTERN = re.compile(
    r"//\s*@doc\s*(.*)"
)

COMMENT_LINE_PATTERN = re.compile(
    r"//\s?(.*)"
)

PARAM_DOC_PATTERN = re.compile(
    r"//\s*@param\s+"
    r"([a-zA-Z0-9_]+)\s+(.+)"
)

RETURN_DOC_PATTERN = re.compile(
    r"//\s*@return\s+(.+)"
)

EXAMPLE_PATTERN = re.compile(
    r"//\s*@example\s+(.+)"
)

ENUM_PATTERN = re.compile(
    r"//\s*@enum\s+([a-zA-Z0-9_]+)"
)

VALUE_PATTERN = re.compile(
    r"//\s*@value\s+"
    r"([a-zA-Z0-9_]+)\s+([a-zA-Z0-9_x]+)"
)


PARAM_SIGNATURE_PATTERN = re.compile(
    r"([a-zA-Z0-9_]+):([a-zA-Z0-9_]+)(?:=.*)?"
)


# =============================================================================
# Types
# =============================================================================

TYPE_MAP = {

    "uint8": "integer",
    "uint16": "integer",
    "uint32": "integer",
    "uint64": "integer",

    "int8": "integer",
    "int16": "integer",
    "int32": "integer",
    "int64": "integer",
    "int": "integer",

    "float": "number",
    "double": "number",

    "bool": "boolean",

    "string": "string",

    "void": "nil",
    
    "table": "table",
}


# =============================================================================
# Containers
# =============================================================================

api = {}

enums = {}


# =============================================================================
# Parse
# =============================================================================

print("[LuaStubGen] Scanning bindings...")


for file in sorted(BINDINGS_DIR.glob("*Bindings.cpp")):

    print(f"[LuaStubGen] Reading {file.name}")

    with file.open(
        "r",
        encoding="utf-8"
    ) as f:

        lines = f.readlines()


    pending_doc = None
    pending_params = {}
    pending_return = None
    pending_examples = []

    current_enum = None


    for line_number, line in enumerate(
        lines,
        start=1
    ):

        raw = line.rstrip()


        # ---------------------------------------------------------
        # Enum start
        # ---------------------------------------------------------

        enum_match = ENUM_PATTERN.search(raw)

        if enum_match:

            current_enum = enum_match.group(1)
            enums[current_enum] = []

            continue



        # ---------------------------------------------------------
        # Enum values
        # ---------------------------------------------------------

        value_match = VALUE_PATTERN.search(raw)

        if value_match and current_enum:

            enums[current_enum].append(
                {
                    "name": value_match.group(1),
                    "value": value_match.group(2)
                }
            )

            continue



        # ---------------------------------------------------------
        # Documentation block
        # ---------------------------------------------------------

        comment = COMMENT_LINE_PATTERN.match(raw)


        if comment:

            text = comment.group(1)


            # new doc block
            m = DOC_PATTERN.match(raw)

            if m:

                pending_doc = m.group(1).strip()

                continue


            # doc continuation
            if pending_doc is not None:

                if text and not text.startswith("@"):

                    pending_doc += "\n" + text.strip()

                    continue



            # param
            m = PARAM_DOC_PATTERN.match(raw)

            if m:

                pending_params[m.group(1)] = m.group(2).strip()

                continue



            # return
            m = RETURN_DOC_PATTERN.match(raw)

            if m:

                pending_return = m.group(1).strip()

                continue



            # example
            m = EXAMPLE_PATTERN.match(raw)

            if m:

                pending_examples.append(
                    m.group(1).strip()
                )

                continue



        # ---------------------------------------------------------
        # Lua API
        # ---------------------------------------------------------

        match = LUA_PATTERN.search(raw)

        if not match:
            continue


        full_name = match.group(1)

        params = match.group(2)

        return_type = match.group(3)



        if "." not in full_name:

            print(
                f"[LuaStubGen] WARNING "
                f"{file.name}:{line_number} "
                f"Invalid name {full_name}"
            )

            continue



        parts = full_name.split(".")


        namespace = ".".join(parts[:-1])

        function = parts[-1]


        api.setdefault(
            namespace,
            []
        )


        api[namespace].append(
            {
                "name": function,
                "params": params,
                "return": return_type,
                "doc": pending_doc,
                "param_docs": pending_params.copy(),
                "return_doc": pending_return,
                "examples": pending_examples.copy()
            }
        )


        pending_doc = None
        pending_params = {}
        pending_return = None
        pending_examples = []


# =============================================================================
# Generate
# =============================================================================

OUTPUT_FILE.parent.mkdir(
    parents=True,
    exist_ok=True
)


print(
    f"[LuaStubGen] Generating {OUTPUT_FILE}"
)


with OUTPUT_FILE.open(
    "w",
    encoding="utf-8"
) as out:


    out.write(
        "---@meta\n\n"
    )

    out.write(
        "-- AUTO GENERATED\n"
        "-- DO NOT EDIT\n\n"
    )


    # -------------------------------------------------------------------------
    # ENUMS
    # -------------------------------------------------------------------------

    for enum_name, values in sorted(
        enums.items()
    ):


        out.write(
            f"---@alias {enum_name} integer\n\n"
        )


        out.write(
            f"{enum_name} = {{\n"
        )


        for item in values:

            out.write(
                f"    {item['name']} = {item['value']},\n"
            )


        out.write(
            "}\n\n"
        )


    # -------------------------------------------------------------------------
    # APIs
    # -------------------------------------------------------------------------
    generated_namespaces = set()

    for namespace in sorted(api):

        parts = namespace.split(".")

        current = ""

        for part in parts:

            if current:
                current += "."

            current += part

            if current not in generated_namespaces:

                out.write(
                    f"---@class {current}\n"
                )

                out.write(
                    f"{current} = {{}}\n\n"
                )

                generated_namespaces.add(current)


        for func in sorted(
            api[namespace],
            key=lambda x: x["name"]
        ):


            if func["doc"]:

                for doc_line in func["doc"].splitlines():

                    out.write(
                        f"--- {doc_line}\n"
                    )

                out.write("---\n")

            if func["examples"]:

                out.write(
                    "---\n"
                    "--- Example:\n"
                )

                for ex in func["examples"]:

                    out.write(
                        f"--- {ex}\n"
                    )


            params = []


            if func["params"]:


                for p in func["params"].split(","):


                    p = p.strip()


                    if not p:
                        continue


                    m = PARAM_SIGNATURE_PATTERN.match(p)


                    if not m:
                        continue


                    name = m.group(1)
                    typ = m.group(2)


                    if typ in TYPE_MAP:

                        lua_type = TYPE_MAP[typ]

                    elif typ in enums:

                        lua_type = typ

                    else:

                        lua_type = "any"



                    doc = func["param_docs"].get(
                        name
                    )


                    if doc:

                        out.write(
                            f"---@param "
                            f"{name} "
                            f"{lua_type} "
                            f"{doc}\n"
                        )

                    else:

                        out.write(
                            f"---@param "
                            f"{name} "
                            f"{lua_type}\n"
                        )


                    params.append(name)



            if func["return"]:

                ret = func["return"]


                if ret != "void":


                    if ret in TYPE_MAP:

                        lua_ret = TYPE_MAP[ret]

                    elif ret in enums:

                        lua_ret = ret

                    else:

                        lua_ret = "any"



                    out.write(
                        f"---@return {lua_ret}"
                    )


                    if func["return_doc"]:

                        out.write(
                            f" {func['return_doc']}"
                        )


                    out.write(
                        "\n"
                    )



            out.write(
                f"function "
                f"{namespace}."
                f"{func['name']}"
                f"({', '.join(params)}) end\n\n"
            )


# =============================================================================
# Done
# =============================================================================

print(
    "[LuaStubGen] Generated "
    f"{sum(len(v) for v in api.values())} functions, "
    f"{len(enums)} enums, "
    f"{len(api)} namespaces"
)