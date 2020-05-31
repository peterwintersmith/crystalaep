// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace AsmXmlTableGenerator
{
    enum OperandDirection
    {
        Source = 0x010000,
        Destination = 0x020000,
        INVALID_DIRECTION = 0xff0000
    }

    enum AddressingMode
    {
        AM_A = 0x0100,
        AM_BA = 0x0200,
        AM_BB = 0x0300,
        AM_BD = 0x0400,
        AM_C = 0x0500,
        AM_D = 0x0600,
        AM_E = 0x0700,
        AM_ES = 0x0800,
        AM_EST = 0x0900,
        AM_F = 0x0a00,
        AM_G = 0x0b00,
        AM_H = 0x0c00,
        AM_I = 0x0d00,
        AM_J = 0x0e00,
        AM_M = 0x0f00,
        AM_N = 0x1000,
        AM_O = 0x1100,
        AM_P = 0x1200,
        AM_Q = 0x1300,
        AM_R = 0x1400,
        AM_S = 0x1500,
        AM_SC = 0x1600,
        AM_T = 0x1700,
        AM_U = 0x1800,
        AM_V = 0x1900,
        AM_W = 0x1a00,
        AM_X = 0x1b00,
        AM_Y = 0x1c00,
        AM_Z = 0x1d00,
        AM_INVALID_MARKER = 0xff00
    }

    enum OperandType
    {
        OT_a = 0x01,
        OT_b = 0x02,
        OT_bcd = 0x03,
        OT_bs = 0x04,
        OT_bsq = 0x05,
        OT_bss = 0x06,
        OT_c = 0x07,
        OT_d = 0x08,
        OT_di = 0x09,
        OT_dq = 0x0a,
        OT_dqp = 0x0b,
        OT_dr = 0x0c,
        OT_ds = 0x0d,
        OT_e = 0x0e,
        OT_er = 0x0f,
        OT_p = 0x10,
        OT_pi = 0x11,
        OT_pd = 0x12,
        OT_ps = 0x13,
        OT_psq = 0x14,
        OT_pt = 0x15,
        OT_ptp = 0x16,
        OT_q = 0x17,
        OT_qi = 0x18,
        OT_qp = 0x19,
        OT_s = 0x1a,
        OT_sd = 0x1b,
        OT_si = 0x1c,
        OT_sr = 0x1d,
        OT_ss = 0x1e,
        OT_st = 0x1f,
        OT_stx = 0x20,
        OT_t = 0x21,
        OT_v = 0x22,
        OT_vds = 0x23,
        OT_vq = 0x24,
        OT_vqp = 0x25,
        OT_vs = 0x26,
        OT_w = 0x27,
        OT_wi = 0x28,
        OT_INVALID_MARKER = 0xff
    }

    struct OpdSrcOrDst
    {
        public UInt32 Flags;
        public string ExactOpnd;
    }
    
    class Instruction
    {
        public Instruction() { }
        public Instruction(Instruction source)
        {
            PrimaryOpcode = source.PrimaryOpcode;
            Prefix = source.Prefix;
            Prefix0F = source.Prefix0F;
            SecondaryOpcode = source.SecondaryOpcode;
            OpcodeExtension = source.OpcodeExtension;
            Direction = source.Direction;
            OpSize = source.OpSize;
            SignExt = source.SignExt;
            ModRmRegAndRm = source.ModRmRegAndRm;
            LockPrefixValid = source.LockPrefixValid;
            Mnemonic = source.Mnemonic;
            Operand1 = source.Operand1;
            Operand2 = source.Operand2;
            Operand3 = source.Operand3;
            Operand4 = source.Operand4;
            Hint = source.Hint;
        }

        public byte PrimaryOpcode; // mandatory
        public byte? Prefix = null;
        public bool Prefix0F; // mandatory
        public byte? SecondaryOpcode = null;
        public int? OpcodeExtension = null;
        public bool? Direction = null;
        public bool? OpSize = null;
        public bool? SignExt = null;
        public bool? ModRmRegAndRm = null;
        public bool? LockPrefixValid = null;
        public string Mnemonic = null;  // mandatory
        public OpdSrcOrDst? Operand1;
        public OpdSrcOrDst? Operand2;
        public OpdSrcOrDst? Operand3;
        public OpdSrcOrDst? Operand4;
        public string Hint = null; // not mandatory but would be nice
    }

    class Program
    {
        const UInt32 DR_MASK = 0xff0000;
        const UInt32 AM_MASK = 0x00ff00;
        const UInt32 OT_MASK = 0x0000ff;
        
        static AddressingMode ConvertAddressingMode(string mode)
        {
            foreach (string name in Enum.GetNames(typeof(AddressingMode)))
            {
                if (name.Substring(3) == mode)
                {
                    return (AddressingMode)Enum.Parse(typeof(AddressingMode), name);
                }
            }

            return AddressingMode.AM_INVALID_MARKER;
        }

        static OperandType ConvertOperandType(string type)
        {
            foreach (string name in Enum.GetNames(typeof(OperandType)))
            {
                if (name.Substring(3) == type)
                {
                    return (OperandType)Enum.Parse(typeof(OperandType), name);
                }
            }

            return OperandType.OT_INVALID_MARKER;
        }

        static byte ParseByte(string value)
        {
            return byte.Parse(
                value,
                System.Globalization.NumberStyles.HexNumber
                );
        }

        static bool InstructionsMatch(Instruction instr1, Instruction instr2)
        {
            OpdSrcOrDst?[] opdsinstr1 = { instr1.Operand1, instr1.Operand2, instr1.Operand3, instr1.Operand4 };
            OpdSrcOrDst?[] opdsinstr2 = { instr2.Operand1, instr2.Operand2, instr2.Operand3, instr2.Operand4 };

            if (!(instr1.Mnemonic == instr2.Mnemonic && instr1.PrimaryOpcode == instr2.PrimaryOpcode &&
                instr1.SecondaryOpcode == instr2.SecondaryOpcode && instr1.OpcodeExtension == instr2.OpcodeExtension))
                return false;

            for (int i = 0; i < 4; i++)
            {
                if (opdsinstr1[i] == null && opdsinstr2[i] == null)
                    break;

                if (opdsinstr1[i] == null && opdsinstr2[i] != null)
                    return false;

                if (opdsinstr1[i] != null && opdsinstr2[i] == null)
                    return false;

                if (opdsinstr1[i].Value.Flags != opdsinstr2[i].Value.Flags)
                    return false;
            }

            return true;
        }

        static bool SpecialCaseOpcode(Instruction ins)
        {
            bool handled = true;

            switch (ins.PrimaryOpcode)
            {
                case 0xf3:
                    // REPE/REPZ prefix
                    break;
                case 0xf2:
                    // REPNE/REPNZ prefix
                    break;
                case 0xf0:
                    // LOCK prefix
                    break;
                case 0x67:
                    // address size replacement prefix
                    break;
                case 0x66:
                    // operand size replacement prefix
                    break;
                case 0x2e:
                    // segment replacement prefixes follow
                    // CS
                    break;
                case 0x36:
                    break;
                case 0x3e:
                    break;
                case 0x26:
                    break;
                case 0x64:
                    break;
                case 0x65:
                    break;
                case 0x0f:
                    // do nothing: this is handled as a prefix later
                    break;
                case 0x40:
                    // 0x40 - 0x48 (inc EAX-EDI) - process here
                    handled = false;
                    break;
                case 0x48:
                    // 0x48 - 0x4f (dec EAX-EDI) - process here
                    handled = false;
                    break;
                case 0x50:
                    // 0x50 - 0x57 (push EAX-EDI) - process here
                    handled = false;
                    break;
                case 0x58:
                    // 0x58 - 0x5f (pop EAX-EDI) - process here
                    handled = false;
                    break;
                // there may be more - beware
                default:
                    handled = false;

                    break;
            }

            return handled;
        }

        static List<Instruction> TableOfInstructions = new List<Instruction>();

        static List<string> uniqueExactOperands = new List<string>();

        static void ProcessXmlReference(string xmlFile)
        {
            XmlReaderSettings readerSettings = new XmlReaderSettings();

            readerSettings.IgnoreComments = true;
            readerSettings.IgnoreWhitespace = true;
            readerSettings.DtdProcessing = DtdProcessing.Ignore;

            XmlReader reader = XmlReader.Create(new FileStream(xmlFile, FileMode.Open), readerSettings);

            bool twoByteOpcode = false, advanceNext = false, foundSyntax = false;
            int operandNum = 0;
            Instruction newInstruction = null;
            byte? lastPrimaryOpcode = null;

            List<Instruction> instrCopies = new List<Instruction>();

            while (reader.Read())
            {
Reparse_Last_Element:
                bool fromSource = false;

                switch (reader.NodeType)
                {
                    case XmlNodeType.Element:

                        if (advanceNext)
                            break;

                        string attribute = null;

                        switch (reader.Name)
                        {
                            case "one-byte":
                                // we're processing one-byte opcodes
                                twoByteOpcode = false;
                                break;
                            case "two-byte":
                                // we're processing two byte opcodes
                                twoByteOpcode = true;
                                break;
                            case "pri_opcd":

                                // new opcode, create new opcode instance
                                
                                // get "value" attribute
                                attribute = reader.GetAttribute("value");
                                if (attribute == null)
                                    throw new Exception("Primary opcode entry doesn't define an opcode value");

                                lastPrimaryOpcode = ParseByte(attribute);

                                //if (lastPrimaryOpcode == 0x65) System.Diagnostics.Debugger.Break();

                                break;
                            case "sec_opcd":
                                
                                reader.Read();

                                if (reader.NodeType != XmlNodeType.Text)
                                    throw new Exception("Secondary opcode entry doesn't define an opcode value");

                                newInstruction.SecondaryOpcode = ParseByte(reader.Value);

                                break;
                            case "pref":
                                
                                reader.Read();

                                if (reader.NodeType != XmlNodeType.Text)
                                    throw new Exception("Prefix doesn't seem to contain valid text node");

                                newInstruction.Prefix = ParseByte(reader.Value);

                                break;
                            case "entry":
                                // may have following optional attributes: Direction, OpSize, ModRmRegAndRm, LockPrefixValid

                                // create new Instruction here as each entry (+ opcode extn) represents a new instruction

                                if (reader.GetAttribute("attr") == "undef" || reader.GetAttribute("is_undoc") == "yes") // || reader.GetAttribute("mode") == "e")
                                {
                                    advanceNext = true;
                                    break;
                                }

                                newInstruction = new Instruction();

                                newInstruction.Prefix0F = twoByteOpcode;

                                if (lastPrimaryOpcode == null)
                                    throw new Exception("No primary opcode found");

                                newInstruction.PrimaryOpcode = lastPrimaryOpcode.Value;

                                operandNum = 0;

                                if (twoByteOpcode == false && SpecialCaseOpcode(newInstruction) == true)
                                {
                                    advanceNext = true;
                                    break;
                                }
                                // else: standard opcode - continue to process here

                                if ((attribute = reader.GetAttribute("direction")) != null)
                                {
                                    newInstruction.Direction = int.Parse(attribute) == 1;
                                }

                                if ((attribute = reader.GetAttribute("op_size")) != null)
                                {
                                    newInstruction.OpSize = int.Parse(attribute) == 1;
                                }

                                if ((attribute = reader.GetAttribute("sign-ext")) != null)
                                {
                                    newInstruction.SignExt = int.Parse(attribute) == 1;
                                }

                                if ((attribute = reader.GetAttribute("r")) != null)
                                {
                                    newInstruction.ModRmRegAndRm = attribute == "yes";
                                }

                                if ((attribute = reader.GetAttribute("lock")) != null)
                                {
                                    newInstruction.LockPrefixValid = attribute == "yes";
                                }

                                // and "mode", let's ignore it unless we later require it

                                break;
                            case "syntax":
                                // alternative ways of rendering the decoded instruction
                                /*
                                if (foundSyntax)
                                {
                                    while (reader.Read())
                                    {
                                        switch (reader.NodeType)
                                        {
                                            case XmlNodeType.EndElement:

                                                // move past syntax element
                                                if (reader.Name == "syntax")
                                                {
                                                    reader.Read();
                                                    goto Reparse_Last_Element;
                                                }

                                                break;
                                        }
                                    }
                                }
                                
                                foundSyntax = true;
                                */

                                operandNum = 0;

                                newInstruction.Operand1 = null;
                                newInstruction.Operand2 = null;
                                newInstruction.Operand3 = null;
                                newInstruction.Operand4 = null;

                                break;
                            case "mnem":

                                reader.Read(); // caught by check below either way

                                if (reader.NodeType != XmlNodeType.Text)
                                    throw new Exception("Mnemonic doesn't seem to contain valid text node");

                                newInstruction.Mnemonic = reader.Value;

                                //if (reader.Value == "FADD")
                                //    System.Diagnostics.Debugger.Break();

                                break;
                            case "dst":

                                //if (newInstruction.Mnemonic == "POP")
                                //    System.Diagnostics.Debugger.Break();

                                OpdSrcOrDst opd = new OpdSrcOrDst();

                                opd.Flags = (UInt32)(fromSource ? OperandDirection.Source : OperandDirection.Destination);
                                opd.ExactOpnd = null;

                                bool foundAM = false, foundOT = false, foundExact = false, termLoop = false;
                                string attribOT = reader.GetAttribute("type");
                                bool showImplicit = reader.GetAttribute("displayed") != "no";

                                while (reader.Read())
                                {
                                    switch (reader.NodeType)
                                    {
                                        case XmlNodeType.Element:
                                            if (reader.Name == "a")
                                            {
                                                reader.Read();

                                                if (reader.NodeType != XmlNodeType.Text)
                                                    throw new Exception("Addressing mode doesn't seem to contain valid text node");

                                                if ((opd.Flags & AM_MASK) != 0)
                                                    System.Diagnostics.Debugger.Break();

                                                opd.Flags |= (UInt32)ConvertAddressingMode(reader.Value);

                                                if ((opd.Flags & (int)AddressingMode.AM_INVALID_MARKER) == (int)AddressingMode.AM_INVALID_MARKER)
                                                    throw new Exception("Invalid addressing mode");

                                                foundAM = true;
                                            }
                                            else if (reader.Name == "t")
                                            {
                                                reader.Read();

                                                if (reader.NodeType != XmlNodeType.Text)
                                                    throw new Exception("Operand type doesn't seem to contain valid text node");

                                                if ((opd.Flags & OT_MASK) != 0)
                                                    System.Diagnostics.Debugger.Break();

                                                opd.Flags |= (UInt32)ConvertOperandType(reader.Value);

                                                if ((opd.Flags & (int)OperandType.OT_INVALID_MARKER) == (int)AddressingMode.AM_INVALID_MARKER)
                                                    throw new Exception("Invalid operand type");

                                                foundOT = true;
                                            }

                                            break;
                                        case XmlNodeType.EndElement:

                                            if (reader.Name == "src" || reader.Name == "dst")
                                            {
                                                termLoop = true;
                                            }

                                            break;
                                        case XmlNodeType.Text:
                                            // implicit operand

                                            //string exact = reader.Value.Replace("r", "E");

                                            //if (!uniqueExactOperands.Contains(reader.Value))
                                            //    uniqueExactOperands.Add(reader.Value);

                                            switch (reader.Value)
                                            {
                                                case "RFlags":
                                                case "Flags":
                                                case "eFlags":
                                                case "SS:[rSP]":
                                                    break;
                                                default:
                                                    
                                                    opd.ExactOpnd = reader.Value;
                                                    foundExact = true;

                                                    if (!String.IsNullOrEmpty(attribOT))
                                                    {
                                                        if ((opd.Flags & OT_MASK) != 0)
                                                            System.Diagnostics.Debugger.Break();

                                                        opd.Flags |= (UInt32)ConvertAddressingMode(attribOT);
                                                        foundAM = true;
                                                    }

                                                    break;
                                            }

                                            break;
                                    }

                                    if (termLoop || (foundAM && foundOT))
                                        break;
                                }

                                // sometimes the text of the src or dst node indicates an implicit operand
                                //if (!foundAM || !foundOT)
                                //    throw new Exception("Found dst operand but not address mode and operand type");

                                if ((opd.Flags & OT_MASK) == 0)
                                    opd.Flags |= (UInt32)OperandType.OT_INVALID_MARKER;

                                if ((opd.Flags & AM_MASK) == 0)
                                    opd.Flags |= (UInt32)AddressingMode.AM_INVALID_MARKER;

                                if (operandNum == 0)
                                    newInstruction.Operand1 = opd;
                                else if (operandNum == 1)
                                    newInstruction.Operand2 = opd;
                                else if (operandNum == 2)
                                    newInstruction.Operand3 = opd;
                                else if (operandNum == 3)
                                    newInstruction.Operand4 = opd;
                                else
                                    throw new Exception(string.Format("Too many operands found for mnemonic {0}", newInstruction.Mnemonic));

                                //if (showImplicit && (foundExact || (foundAM && foundOT)))
                                if (showImplicit && (foundExact || (foundAM || foundOT)))
                                {
                                    // advance for implicit operand or valid AM/OT combo
                                    operandNum++;
                                }

                                break;
                            case "src":
                                fromSource = true;
                                goto case "dst";
                            case "opcd_ext":

                                reader.Read();

                                if (reader.NodeType != XmlNodeType.Text)
                                    throw new Exception("Opcode extension doesn't seem to contain valid text node");

                                newInstruction.OpcodeExtension = int.Parse(reader.Value);

                                break;
                            case "brief":
                                reader.Read();

                                if (reader.NodeType == XmlNodeType.Text)
                                {
                                    newInstruction.Hint = reader.Value;
                                }
                                
                                break;
                        }

                        break;
                    case XmlNodeType.Text:
                        // no special handling required

                        break;
                    case XmlNodeType.EndElement:
                        
                        if(reader.Name == "pri_opcd")
                        {
                            lastPrimaryOpcode = null;
                        }
                        else if (reader.Name == "entry")
                        {
                            if (!advanceNext)
                            {
                                // was handled by SpecialCaseOpcode(...)
                                int insCount = TableOfInstructions.Count;

                                if(insCount != 0)
                                {
                                    if (TableOfInstructions[insCount - 1].PrimaryOpcode == newInstruction.PrimaryOpcode)
                                    {
                                        if (newInstruction.SecondaryOpcode == null && newInstruction.OpcodeExtension == null && (TableOfInstructions[insCount - 1].Prefix0F == newInstruction.Prefix0F))
                                        {
                                            // trying to add an existing instruction with new syntax, let's not allow that for now
                                            
                                            newInstruction = null;
                                            advanceNext = false;
                                            foundSyntax = false;

                                            break;
                                        }
                                    }
                                }

                                foreach (Instruction instr in instrCopies)
                                {
                                    instr.Hint = newInstruction.Hint;

                                    if ((twoByteOpcode == true || !SpecialCaseOpcode(instr)) && (TableOfInstructions.Count == 0 || !InstructionsMatch(instr, TableOfInstructions[TableOfInstructions.Count - 1])))
                                        TableOfInstructions.Add(instr);
                                }

                                instrCopies.Clear();
                            }

                            newInstruction = null;
                            advanceNext = false;
                            foundSyntax = false;
                        }
                        else if (reader.Name == "syntax")
                        {
                            //if (newInstruction == null) System.Diagnostics.Debugger.Break();
                            if (newInstruction == null)
                            {
                                //Console.WriteLine("Didn't add {0} {1:X2}", TableOfInstructions[TableOfInstructions.Count - 1].Mnemonic, TableOfInstructions[TableOfInstructions.Count - 1].PrimaryOpcode);
                            }
                            else
                            {
                                Instruction instrCpy = new Instruction(newInstruction);
                                instrCopies.Add(instrCpy);
                            }
                        }

                        break;
                    default:
                        break;
                }
            }
        }

        static void RenderInstructionsTableCPlusPlus(bool printable)
        {
            /*
                class Instruction
                {
                    public byte PrimaryOpcode; // mandatory
                    public byte? Prefix = null;
                    public bool? Prefix0F = null;
                    public byte? SecondaryOpcode = null;
                    public int? OpcodeExtension = null;
                    public bool? Direction = null;
                    public bool? OpSize = null;
                    public bool? ModRmRegAndRm = null;
                    public bool? LockPrefixValid = null;
                    public string Mnemonic = null;  // mandatory
                    public OpdSrcOrDst? Operand1;
                    public OpdSrcOrDst? Operand2;
                    public OpdSrcOrDst? Operand3;
                    public OpdSrcOrDst? Operand4;
                    public string Hint = null; // not mandatory but would be nice
                }
            
             */

            Console.WriteLine("struct {");
            
            System.Reflection.FieldInfo[] fields = typeof(Instruction).GetFields();

            foreach (System.Reflection.FieldInfo fi in fields)
            {
                string type = fi.FieldType.ToString().ToUpper();
                Console.WriteLine("\t{0} {1};", type, fi.Name);
            }

            Console.WriteLine("}} TableOfInstructions[{0}] = {{", TableOfInstructions.Count);

            foreach (Instruction ins in TableOfInstructions)
            {
                Console.Write("\t{ ");

                Console.Write("0x{0:X2}, 0x{1:X2}, {2}, 0x{3:X2}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, ",
                    ins.PrimaryOpcode,
                    ins.Prefix ?? 0,
                    ins.Prefix0F.ToString().ToUpper(),
                    ins.SecondaryOpcode ?? 0,
                    ins.OpcodeExtension ?? -1,
                    ins.Direction.HasValue ? Convert.ToInt32(ins.Direction.Value) : -1,
                    ins.OpSize.HasValue ? Convert.ToInt32(ins.OpSize.Value) : -1,
                    ins.SignExt.HasValue ? Convert.ToInt32(ins.SignExt.Value) : -1,
                    ins.ModRmRegAndRm.HasValue ? Convert.ToInt32(ins.ModRmRegAndRm.Value) : -1,
                    ins.LockPrefixValid.HasValue ? Convert.ToInt32(ins.LockPrefixValid.Value) : -1,
                    "\"" + ins.Mnemonic + "\"" 
                    );

                // now add operands

                OpdSrcOrDst?[] operands = new OpdSrcOrDst?[4] { ins.Operand1, ins.Operand2, ins.Operand3, ins.Operand4 };

                if (printable)
                {
                    foreach (OpdSrcOrDst? opd in operands)
                    {
                        if (opd != null)
                        {
                            Console.Write("{{ {0}, {1}, {2}, {3} }}, ",
                                ((OperandDirection)(opd.Value.Flags & DR_MASK)).ToString(),
                                ((AddressingMode)(opd.Value.Flags & AM_MASK)).ToString(),
                                ((OperandType)(opd.Value.Flags & OT_MASK)).ToString(),
                                opd.Value.ExactOpnd == null ? "NULL" : "\"" + opd.Value.ExactOpnd + "\"");
                        }
                        else
                        {
                            Console.Write("NO_OPERAND, ");
                        }
                    }
                }
                else
                {
                    foreach (OpdSrcOrDst? opd in operands)
                    {
                        if (opd != null)
                        {
                            Console.Write("{{ 0x{0}, {1} }}, ", opd.Value.Flags.ToString("X6"), opd.Value.ExactOpnd == null ? "NULL" : "\"" + opd.Value.ExactOpnd + "\"");
                        }
                        else
                        {
                            Console.Write("{0xffffff, NULL}, ");
                        }
                    }
                }
                
                //Console.Write("{0}", ins.Hint != null ? "\"" + ins.Hint + "\"" : "NULL");
                Console.Write("NULL");

                Console.WriteLine(" },");
            }

            Console.WriteLine("};");
        }

        static void RenderSingleInstruction(string mnem)
        {
            System.Reflection.FieldInfo[] fields = typeof(Instruction).GetFields();

            Console.WriteLine("Instruction:");

            foreach (System.Reflection.FieldInfo fi in fields)
            {
                string type = fi.FieldType.ToString();
                Console.WriteLine("\t{0} {1};", type, fi.Name);
            }

            int idx = 0;

            foreach (Instruction ins in TableOfInstructions)
            {
                idx++;

                if (mnem.ToLower() != ins.Mnemonic.ToLower())
                    continue;

                Console.Write("[idx {0}] ", idx - 1);

                Console.Write("0x{0:X2}, 0x{1:X2}, {2}, 0x{3:X2}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, ",
                    ins.PrimaryOpcode,
                    ins.Prefix ?? 0,
                    ins.Prefix0F.ToString().ToUpper(),
                    ins.SecondaryOpcode ?? 0,
                    ins.OpcodeExtension ?? -1,
                    ins.Direction.HasValue ? Convert.ToInt32(ins.Direction.Value) : -1,
                    ins.OpSize.HasValue ? Convert.ToInt32(ins.OpSize.Value) : -1,
                    ins.SignExt.HasValue ? Convert.ToInt32(ins.SignExt.Value) : -1,
                    ins.ModRmRegAndRm.HasValue ? Convert.ToInt32(ins.ModRmRegAndRm.Value) : -1,
                    ins.LockPrefixValid.HasValue ? Convert.ToInt32(ins.LockPrefixValid.Value) : -1,
                    "\"" + ins.Mnemonic + "\""
                    );

                // now add operands

                OpdSrcOrDst?[] operands = new OpdSrcOrDst?[4] { ins.Operand1, ins.Operand2, ins.Operand3, ins.Operand4 };

                foreach (OpdSrcOrDst? opd in operands)
                {
                    if (opd != null)
                    {
                        Console.Write("{{ {0}, {1}, {2}, {3} }}, ",
                            ((OperandDirection)(opd.Value.Flags & DR_MASK)).ToString(),
                            ((AddressingMode)(opd.Value.Flags   & AM_MASK)).ToString(),
                            ((OperandType)(opd.Value.Flags      & OT_MASK)).ToString(),
                            opd.Value.ExactOpnd == null ? "NULL" : "\"" + opd.Value.ExactOpnd + "\"");
                    }
                    else
                    {
                        Console.Write("NO_OPERAND, ");
                    }
                }

                //Console.Write("{0}", ins.Hint != null ? "\"" + ins.Hint + "\"" : "NULL");
                Console.WriteLine();
            }

            Console.WriteLine();
        }

        static void Main(string[] args)
        {
            ProcessXmlReference(@"C:\Work\ebooks\x86reference.xml");

            if (args.Length == 0 || (args.Length != 0 && args[0] == "print"))
            {
                bool printable = false;

                if (args.Length != 0 && args[0] == "print")
                    printable = true;

                RenderInstructionsTableCPlusPlus(printable);

                foreach (string exact in uniqueExactOperands)
                {
                    //Console.WriteLine(exact);
                }
            }
            else
            {
                if (args[0] == "am-unique")
                {
                    List<string> uniqueAMs = new List<string>();

                    foreach (Instruction ins in TableOfInstructions)
                    {
                        OpdSrcOrDst?[] opds = { ins.Operand1, ins.Operand2, ins.Operand3, ins.Operand4 };


                        for (int i = 0; i < 4 && opds[i] != null; i++)
                        {
                            string amstr = ((AddressingMode)(opds[i].Value.Flags & AM_MASK)).ToString();

                            if (!uniqueAMs.Contains(amstr))
                                uniqueAMs.Add(amstr);
                        }
                    }

                    foreach (string am in uniqueAMs)
                        Console.WriteLine(am);
                }
                else if (args[0] == "ot-unique")
                {
                    List<string> uniqueOTs = new List<string>();

                    foreach (Instruction ins in TableOfInstructions)
                    {
                        OpdSrcOrDst?[] opds = { ins.Operand1, ins.Operand2, ins.Operand3, ins.Operand4 };


                        for (int i = 0; i < 4 && opds[i] != null; i++)
                        {
                            string amstr = ((OperandType)(opds[i].Value.Flags & OT_MASK)).ToString();

                            if (!uniqueOTs.Contains(amstr))
                                uniqueOTs.Add(amstr);
                        }
                    }

                    foreach (string am in uniqueOTs)
                        Console.WriteLine(am);
                }
                else
                {
                    RenderSingleInstruction(args[0]);
                }
            }
        }
    }
}
