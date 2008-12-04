#!/usr/bin/env python

"""
    A simple test client for the lua debugger.

    Copyright 2009 Sri Panyam

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

Version:
    Sri Panyam      05 Dec 2008
    Initial version
"""

import os, sys
import re, string
import socket
import simplejson as json
import Queue, threading

def strip_quotes(x):
    if x[0] == "'" or x[0] == '"':
        x = x[1:]
    if x[-1] == "'" or x[-1] == '"':
        x = x[:-1]
    return x


def tokenize(string):
    """ A simple tokenizer for getting contigious letters/alphabets 
    or a quoted string. """
    exp = re.compile('([^\s\"\']+)|(\"[^\"]*\")|(\'[^\']*\')')

    tokens = exp.finditer(string)

    return [token.group() for token in tokens]

class Debugger:
    """ The actual debugger front end. """

    PROMPT = "LDB: "

    def __init__(self, luahost = "localhost", luaport = 9999):
        self.current_context    = -1
        self.context_list       = []
        self.currContext        = None
        self.currFrame          = 0
        self.luaHost            = luahost
        self.luaPort            = luaport
        self.msg_counter        = 0
        self.messages           = Queue.Queue()
        self.debugClient        = DebugClient(self.message_callback)

    def __del__(self):
        self.disconnect()

    def do_prompt(self, prompt = PROMPT):
        if self.debugClient.isAlive():
            return raw_input(prompt).strip()
        return None

    def run(self):
        """ Runs the command line loop. """
        
        if self.debugClient.isAlive():
            self.debugClient.disconnect()

        self.debugClient.connect()
        self.debugClient.start()

        try:
            input = self.do_prompt()
            while (input != None and input != "quit"):
                if input:
                    self.process_input(input)
                input = self.do_prompt()
        except:
            a, b, c = sys.exc_info()
            print "Exception at Debugger: ", a, b, dir(c)

        self.disconnect()

    def send_message(self, cmd, data = None):
        self.msg_counter += 1
        self.debugClient.send_message(self.msg_counter, cmd, data)

    def process_input(self, input):
        args        = tokenize(input)
        cmd         = args[0]

        handler     = self.get_command_handler(cmd)

        handler(cmd, args[1:], True)

    def get_command_handler(self, cmd):
        return getattr(self, "command_" + cmd, self.invalid_command)

    def disconnect(self):
        print "Stopping Debugger ..."
        self.debugClient.disconnect()

    def message_callback(self, data):
        # print "Decoding data: ", data
        decdata = json.decoder.JSONDecoder().decode(data)
        # print "Decoded data: ", decdata

        msgtype = decdata["type"]

        if msgtype == "Reply":
            # We are dealing with a reply instead of an event
            if 'code' not in decdata:
                print "Code not found in reply: ", decdata
                return

            if decdata['code'] != 0:
                print "Code: ", decdata['code'], " Error: ", decdata['value']
                return

            orig_msg    = decdata["original"]
            handler     = None
            command     = None
            if 'cmd' in orig_msg:
                command = orig_msg['cmd']
            if command:
                handler = self.get_command_handler(command)

            if handler:
                handler(command, decdata, False)
            else:
                print "No handler found for command: %s" % command
        elif msgtype == "Event":
            self.handle_event(decdata["event"], decdata["data"])
        else:
            print "Invalid message type: ", msgtype, ", Data: ", decdata

    def handle_event(self, evt_name, evt_data):
        """ Handles an event from the server. """
        print "Recieved Event: ", evt_name # , evt_data
        if evt_name == "ContextPaused":
            pass
        else:
            pass

    def command_break(self, command, args, sending):
        """
        Sets a break point.

        Parameters:
            \\1  filename/function  File or Function where breakpoint is to be set
            \\2  linenumber         Line where the breakpoint is to be set
                                    (this implies breakpoint is a file
                                    breakpoint rather than a function
                                    breakpoint).
        """
        if sending:
            if len(args) == 0:
                return self.print_help(command)

            if len(args) == 1:
                data = {'funcname': args[0]}
            else:
                data = {'filename': "@" + args[0], 'linenum': int(args[1])}

            # print "Sending break command: ", data
            self.send_message("break", data)
        else:
            data    = args
            print "Breakpoint successfully set at: ", data['original']['data']

    def command_clear(self, command, args, sending):
        """
        Clears a break point.

        Parameters:
            \\1  filename/function  File or Function where breakpoint is to be cleared
            \\2  linenumber         Line where the breakpoint is to be cleared
                                    (this implies breakpoint is a file
                                    breakpoint rather than a function
                                    breakpoint).
        """
        if sending:
            if len(args) == 0:
                return self.print_help(command)

            if len(args) == 1:
                data = {'funcname': args[0]}
            else:
                data = {'filename': args[0], 'linenum': int(args[1])}

            self.send_message("clear", data)
        else: 
            data    = args
            print "Breakpoint successfully cleared at: ", data

    def command_clearall(self, command, args, sending):
        if sending:
            self.send_message("clearall")
        else: pass

    def command_step(self, command, args, sending):
        if sending: 
            if len(args) == 0:
                return self.print_help(command)

            self.send_message("step", {'context': args[0]})
        else: pass

    def command_next(self, command, args, sending):
        if sending: 
            if len(args) == 0:
                return self.print_help(command)

            self.send_message("next", {'context': args[0]})
        else: pass

    def command_until(self, command, args, sending):
        if sending: pass
        else: pass

    def command_finish(self, command, args, sending):
        if sending: 
            if len(args) == 0:
                return self.print_help(command)

            self.send_message("finish", {'context': args[0]})
        else: pass

    def command_continue(self, command, args, sending):
        """
        Continues from a break point.

        Parameters:
            \\1  context    Address of the context to continue one.  See
                            the 'contexts' command for further info.
        """

        if sending:
            if len(args) == 0:
                return self.print_help(command)

            self.send_message("continue", {'context': args[0]})
        else: pass

    def command_context(self, command, args, sending):
        """
        Sets the current context.  If no context is selected, the current
        context is printed.

        Parameters:
            \\1  index  Index of the context to be used as the current
                        context.  The contexts currently being debugged can
                        be obtained with the contexts command.
        """

        if len(args) > 0:
            self.current_context = int(args[0])

        if self.current_context < 0 or (not self.context_list):
            print "No contexts available.  Please run the contexts command."
            return

        if self.current_context >= len(self.context_list):
            print "Invalid context index..."
            return

        print "Current Context: ", self.context_list[self.current_context]

    def command_contexts(self, command, args, sending):
        if sending:
            self.send_message("contexts")
        else: 
            self.context_list = args["value"]

            if "noprint" not in args["original"]:
                print ""
                print "Contexts:"
                print "========="

                for ctx in self.context_list:
                    print "    Name: '%s', Addr: %s, Running: %s" % (ctx["name"], ctx["address"], str(ctx["running"]))
                    if (not ctx["running"]) and "location" in ctx:
                        print "                At: %s" % ctx["location"]

                print ""

    def command_reset(self, command, args, sending):
        if sending:
            self.send_message("reset")
        else: pass

    def command_print(self, command, args, sending):
        if sending: pass
        else: pass

    def command_list(self, command, args, sending):
        if sending: pass
        else: pass

    def command_set(self, command, args, sending):
        if sending: pass
        else: pass

    def command_eval(self, command, args, sending):
        """
        Evaluates an expression in a particular stack.

        Parameters:
            \\1  context    Address of the context where the expression is
                            to be evaluated.
            \\2  expr_str   Lua expression string to be evaluated.
        """
        if sending:
            if len(args) < 2:
                return self.print_help(command)

            self.send_message("eval", {'context': args[0], 'expr_str': strip_quotes(args[1])})
        else:
            data    = args
            print "Result: ", data["value"]

    def command_local(self, command, args, sending):
        """
        Prints the value of a local variable in a given stack frame.

        Parameters:
            \\1  context    Address of the context whose local variables
                            are to be printed.
            \\2  lvindex    Optional.  The local variable index whose value
                            is to be extracted - Defaults to 1.
            \\3  nlevels    Optional.  Number of levels to recurse into the
                            variable - Defaults to 1.
            \\4  frame      Optional.  The frame whose LVs are to be
                            printed.  Defaults to 0.
        """
        if sending: 
            if len(args) == 0:
                return self.print_help(command)

            lvindex = 1
            nlevels = 1
            frame   = 0

            if len(args) > 1:
                lvindex = int(args[1])
                if len(args) > 2:
                    nlevels = int(args[2])
                    if len(args) > 3:
                        frame   = int(args[3])

            self.send_message("local", {'context': args[0], 'frame': frame, 'lv': lvindex, 'nlevels': nlevels})
        else:
            results     = args["value"]

            local_name  = results["name"]
            local_type  = results["type"]
            local_value = results["value"]

            def print_table(table_value, level = 1):
                for kvpair in table_value:
                    lKey = kvpair['key']
                    lVal = kvpair['value']
                    if lVal["type"] == "table" and "raw" not in lVal:
                        print "%s[%s (%s)]     -> " % (level * "    ", str(lKey["value"]), str(lKey["type"]))
                        print_table(lVal["value"], level + 1)
                    else:
                        print "%s[%s (%s)]     ->      %s : (%s)" %     \
                                        (level * "    ",
                                         str(lKey["value"]), str(lKey["type"]),
                                         str(lVal["value"]), str(lVal["type"]))
                
            print ""
            print "Name: %s, Value: " % local_name, local_type

            if local_type != "table":
                print "Local Value: ", local_value
            else:
                print_table(local_value)

    def command_locals(self, command, args, sending):
        """
        Prints all local variables in a given stack frame.

        Parameters:
            \\1  context    Address of the context whose local variables
                            are to be printed.
            \\2  frame      Optional.  The frame whose LVs are to be
                            printed.  Defaults to 0.
        """

        if sending:
            if len(args) == 0:
                return self.print_help(command)

            if len(args) > 1: frame = int(args[1])
            else: frame = 0

            self.send_message("locals", {'context': args[0], 'frame': frame})
        else: 
            print ""
            print "Locals:"
            print "======="

            for lv in args["value"]:
                print "    Name: '%s'" % (lv)

            print ""

    def command_frame(self, command, args, sending):
        """
        Sets the selected frame as the active frame of the current context.
        This is only valid if the context is being debugged.
        """
        if self.current_context < 0 or self.current_context >= len(self.context_list):
            print "No context selected.  Please run the context command to select a context ..."
            return

        if self.context_list[self.current_context]["running"]:
            print "Context is not paused.  Frame cannot be selected."
            return

    def command_file(self, command, args, sending):
        """
        Retrieves contents of a file in a given range.

        Parameters:
            \\1 filename
            \\2 firstline   (optional - default 1)
            \\3 lastline    (optional - default -1 => last line of the file)

        Returns (via callback):
            List of lines from the file in a given range.
        """
        if sending:
            if len(args) == 0:
                return self.print_help(command)

            data = {'file': args[0]}
            if len(args) > 1:
                data['first'] = int(args[1])
                if len(args) > 2:
                    data['last'] = int(args[2])

            self.send_message("file", data)
        else:
            data    = args

            orig    = data['original']
            file    = orig['data']['file']
            first   = 1
            if 'first' in orig['data']: first = orig['data']['first']

            lines   = data['value']

            print " ---  %s >>" % file

            for i in xrange(0, len(lines)):
                print " %5d > %s" % (first + i, lines[i])

    def command_help(self, command, args, sending):
        if args:
            self.print_help(args[0])
        else:
            self.print_help()


    def invalid_command(self, command, args, sending):
        print "Invalid command: %s" % command

        self.print_help(command, args)

    def print_help(self, command = None, args = []):
        handler = self.invalid_command
        if command:
            handler = self.get_command_handler(command)

        if handler != self.invalid_command:
            print "%s" % command
            print handler.__doc__
        else:
            cmdlen = len("command_")
            print "Command List: "
            for (item, value) in Debugger.__dict__.items():
                if item.startswith("command_"):
                    cmdname     = item[cmdlen : ]
                    handler     = self.get_command_handler(cmdname)

                    print "    %s" % cmdname
                    if handler.__doc__:
                        print handler.__doc__

class DebugClient(threading.Thread):
    """ A class that simply manages connections to the server and sends and
    recives messages. """
    MSG_WAITALL = 0x100

    def sendall(self, messsage, msglen = None):
        totalsent   = 0
        if msglen is None: msglen = len(message)

        while totalsent < msglen:
            sent = self.serverSocket.send(message[totalsent:])
            if sent == 0:
                raise RuntimeError, "Socket connection broken"
            totalsent += sent

    def recieveall(self, msglen):
        msg = ""
        while len(msg) < msglen:
            chunk = self.serverSocket.recv(msglen - len(msg))
            if chunk == "":
                raise RuntimeError, "Socket connection broken"
            msg += chunk
        return msg

    def __init__(self, msg_callback = None):
        self.serverSocket         = None
        self.stopped        = False

        if msg_callback is not None:
            self.msg_callback   = msg_callback
        else:
            self.msg_callback   = self.default_msg_callback

        threading.Thread.__init__(self)

    def default_msg_callback(self, data):
        print "Recieved: ", data

    def __del__(self):
        self.disconnect()

    def connect(self, host = "localhost", port = 9999):
        """ Connect to the debug server. """

        if self.isAlive():
            self.disconnect()

        self.serverSocket     = socket.socket()
        self.serverSocket.connect((host, port))

    def disconnect(self):
        """ Disconnect from the server. """
        if self.isAlive():
            print "Disconnecting from server ..."
            self.stopped = True
            if self.serverSocket:
                print "Closing server socket..."
                self.serverSocket.shutdown(socket.SHUT_RDWR)
                self.serverSocket.close()

        self.serverSocket = None

        if self.isAlive():
            self.join()

    def run(self):
        """ Thread callback function. """
        self.stopped    = False

        if self.serverSocket is None:
            print "Debug client has not been started.  Please 'connect' first."
            return 

        while self.serverSocket is not None and not self.stopped:
            try:
                data = self.read_string()
                if data:
                    self.msg_callback(data)
                else:
                    print "Server closed connection."
                    return 
            except:
                a, b, c = sys.exc_info()
                print "Exception at DebugClient: ", a, b, dir(c)
                return

    def read_string(self):
        data    = self.serverSocket.recv(4, DebugClient.MSG_WAITALL)

        if len(data) == 4:
            datalen = ((ord(data[0]) & 0xff))       |   \
                      ((ord(data[1]) & 0xff) << 8)  |   \
                      ((ord(data[2]) & 0xff) << 16) |   \
                      ((ord(data[3]) & 0xff) << 24)

            return self.serverSocket.recv(datalen, DebugClient.MSG_WAITALL)

        return None

    def send_message(self, id, msg_type, msg_data = None):
        obj_str = json.encoder.JSONEncoder().encode({'id': id, 'cmd': msg_type, 'data': msg_data})
        self.send_string(obj_str)

    def send_string(self, s):
        """ Sends a string s to the server. """
        string_bytes    = s                 # bytearray(s)
        length          = len(string_bytes)
        size            = chr(length & 0xff)            +   \
                          chr((length >> 8) & 0xff)     +   \
                          chr((length >> 16) & 0xff)    +   \
                          chr((length >> 24) & 0xff)
        self.serverSocket.send(size)
        self.serverSocket.send(string_bytes)

def run(): Debugger().run()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Usage: python client.py run"
    else:
        if sys.argv[1] == "run":
            run()

