from __future__ import print_function
from flask import Flask, g, session, redirect, url_for, escape, request, render_template, jsonify
from flask.json import JSONEncoder, dumps
import ConfigParser
from os.path import expanduser
import sys
import re
import os
import urllib
import subprocess
from time import sleep

app = Flask(__name__)

class CustomJSONEncoder(JSONEncoder):
    def default(self, obj):
        if isinstance(obj, User):
            # Implement code to convert Passport object to a dict
            return obj.row
        else:
            JSONEncoder.default(self, obj)

config = ConfigParser.ConfigParser()
config.read(expanduser('~/.amsrc'))

# set the secret key.  keep this really secret:
app.secret_key = 'This needs to be read from the config file'

# Now tell Flask to use the custom class
app.json_encoder = CustomJSONEncoder

"""
    If nothing specified, default to ams.html
"""
@app.route('/')
def index():
    return redirect(url_for('ams'))

"""
    The main page: 
    Select directory and paramaters, and run the amsDecoder suite
"""
@app.route('/ams', methods=['GET','POST'])
def ams():
    return render_template('ams.html')

@app.route('/amsdecode', methods=['GET','POST'])
def amsdecode():
    print("Subdir="+request.values['subdir'])
    print("minFreq="+str(request.values['minFreq']))
    print("maxFreq="+str(request.values['maxFreq']))
    print("Threshold="+str(request.values['threshold']))
    subprocess.call("/usr/local/bin/processBinFiles.py", cwd=('%s' % request.values['subdir']))
    return "ok"

@app.route('/channalise', methods=['GET','POST'])
def channalise():
    subprocess.call("/usr/local/bin/channelise.py", cwd=('%s' % request.values['subdir']))
    return "ok"

@app.route('/licensecheck', methods=['GET','POST'])
def licensecheck():
    print(request.values)
    print("Subdir="+request.values['subdir']);
    print("minFreq="+str(request.values['minFreq']));
    subprocess.call("/usr/local/bin/licenseCheck", cwd=('%s' % request.values['subdir']))
    return "ok"

@app.route('/processopsroom', methods=['GET','POST'])
def processopsroom():
    print(request.values)
    print("Subdir="+request.values['subdir']);
    print("minFreq="+str(request.values['minFreq']));
    subprocess.call("/usr/local/bin/processOpsRoom", cwd=('%s' % request.values['subdir']))
    return "ok"

"""
    This is called via an Ajax call from jqueryFileTree javascript function
    to handle the server side file browsing
"""
@app.route('/jqueryFileTree', methods=['GET', 'POST'])
def dirlist():
    r=['<ul class="jqueryFileTree" style="display: none;">']
    try:
        r=['<ul class="jqueryFileTree" style="display: none;">']
        d = urllib.unquote(request.form['dir']);
        for f in os.listdir(d):
            if f.startswith('.'): continue
            ff=os.path.join(d,f)
            if os.path.isdir(ff):
                r.append('<li class="directory collapsed"><a href="#" rel="%s/">%s</a></li>' % (ff,f))
            else:
                if f.endswith('.bin'):
                    e=os.path.splitext(f)[1][1:] # get .ext and remove dot
                    r.append('<li class="file ext_%s"><a href="#" rel="%s">%s</a></li>' % (e,ff,f))
        r.append('</ul>')
    except Exception,e:
        r.append('Could not load directory: %s' % str(e))
    r.append('</ul>')
    return (''.join(r))


if __name__ == '__main__':
	app.run(host='0.0.0.0', port=8086, debug=True)

