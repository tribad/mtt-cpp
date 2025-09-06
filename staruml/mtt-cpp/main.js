//
// Copyright 2019 Hans-Juergen Lange <hjl@simulated-universe.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the “Software”), to deal in the
// Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//  These are the node.js modules we need for some specials.
const path          = require('path');
const child_process = require('child_process');
//
//  Defining our preferences to use.
var mttcppPreferences = {
  id: "mttcpp",
  name: "mttcpp",
  schema: {
    "mttcpp.gen": {
      text: "mttcpp Generation",
      type: "section"
    },
    "mttcpp.gen.basedir": {
      text: "Output Base Directory",
      description: "This is the directory name to start generating into.",
      type: "string",
      default: ""
    }
  }
}

//
//  This function gets called if either the hot-key or the menu-command gets executed.
function run_mttcpp () {
    //
    //  The actual filename of the project.
    var the_filename = app.project.getFilename();
    //
    //  Check if it set in any way.
    if ((the_filename == null) || (the_filename.length == 0)) {
        //
        //  With the filters set we do not only limit the content of the file box.
        //  The file extension gets automaticly set if we omit it.
        var filters = [ {name : 'Model Files', extensions: ['mdj'] } ];
        //
        //  Start the save dialog
        the_filename = app.dialogs.showSaveDialog('Save File', '.', filters);
        //
        //  Check whether we now have a filename for the model.
        if ((the_filename == null) || (the_filename.length == 0)) {
            //
            //  If not tell the user that we do not generate anything.
            window.alert('File Not Saved. Do not generate');
        } else {
        	   app.preferences.set("mttcpp.gen.basedir", path.basename(the_filename, '.mdj'));
        }
    }
    //
    //  We check again the filename. Because it has been set already or
    //  was set via the save-file-dialog.
    if (the_filename != null) {
        if (the_filename.length > 0) {
            //
            //  Do save the file. Need some exception handling. But later.
            app.project.save(the_filename);
            //
            //  Check if the basedir preference is set.
            var outputbase = app.preferences.get("mttcpp.gen.basedir");
            
            if ((outputbase == null) || (outputbase.length == 0)) {
            	  outputbase = path.basename(the_filename, '.mdj');
	        	     app.preferences.set("mttcpp.gen.basedir", outputbase);
	        	}
				
            //
            //  get the directory we dump the code in.
            var the_dir     = path.dirname(the_filename) + path.sep + outputbase;
            //
            //  Create the command line to execute.
            var the_command = 'mtt-cpp -s2 -d ' + the_dir + ' ' + the_filename;
            console.log(`Command:  ${the_command}`);
            //
            // Do it. We do not have an error handling here.
            child_process.exec(the_command, (err, stdout, stderr) => {
              if (err) {
                // node couldn't execute the command
                return;
              }

              // the *entire* stdout and stderr (buffered)
              console.log(`stdout: ${stdout}`);
              console.log(`stderr: ${stderr}`);
            });
        }
    }
}
//
// This gets called on initializing the extension.
function init () {
   app.commands.register('mttcpp:run_mttcpp', run_mttcpp)
	app.preferences.register(mttcppPreferences)
}
//
//
exports.init = init
