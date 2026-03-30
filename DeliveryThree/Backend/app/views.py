"""
Flask Documentation:     https://flask.palletsprojects.com/
Jinja2 Documentation:    https://jinja.palletsprojects.com/
Werkzeug Documentation:  https://werkzeug.palletsprojects.com/
This file creates your application.
"""

# from crypt import methods
import site 

from app import app, Config,  mongo, Mqtt
from flask import escape, render_template, request, jsonify, send_file, redirect, make_response, send_from_directory 
from json import dumps, loads 
from werkzeug.utils import secure_filename
from datetime import datetime,timedelta, timezone
from os import getcwd
from os.path import join, exists
from time import time, ctime
from math import floor
 

@app.route("/api/latest", methods=["GET"])
def get_latest():
    try:
        db = mongo.get_db()
        if db is None:
            return jsonify({"status": "Failed", "message": "Database connection failed"}), 500

        latest = db["WeatherStation"].find_one(
            {},
            sort=[("timestamp", -1)],
            projection={"_id": 0}
        )

        if latest is None:
            return jsonify({"status": "Failed", "message": "No data found"}), 404

        return jsonify({"status": "Success", "data": latest}), 200

    except Exception as e:
        return jsonify({"status": "Failed", "message": str(e)}), 500
    
    


@app.route("/api/range/<int:start>/<int:end>", methods=["GET"])
def get_range(start, end):
    try:
        db = mongo.get_db()
        if db is None:
            return jsonify({"status": "Failed", "message": "Database connection failed"}), 500

        rows = list(
            db["WeatherStation"].find(
                {"timestamp": {"$gte": start, "$lte": end}},
                {"_id": 0}
            ).sort("timestamp", 1)
        )

        return jsonify({"status": "Success", "data": rows}), 200

    except Exception as e:
        return jsonify({"status": "Failed", "message": str(e)}), 500
    

    
@app.route("/api/stats/<int:start>/<int:end>", methods=["GET"])
def get_stats(start, end):
    try:
        db = mongo.get_db()
        if db is None:
            return jsonify({"status": "Failed", "message": "Database connection failed"}), 500

        match_stage = {
            "$match": {
                "timestamp": {"$gte": start, "$lte": end}
            }
        }

        group_stage = {
            "$group": {
                "_id": None,

                "temperature_min": {"$min": "$temperature_dht_c"},
                "temperature_max": {"$max": "$temperature_dht_c"},
                "temperature_avg": {"$avg": "$temperature_dht_c"},

                "humidity_min": {"$min": "$humidity_pct"},
                "humidity_max": {"$max": "$humidity_pct"},
                "humidity_avg": {"$avg": "$humidity_pct"},

                "moisture_min": {"$min": "$moisture_pct"},
                "moisture_max": {"$max": "$moisture_pct"},
                "moisture_avg": {"$avg": "$moisture_pct"}
            }
        }

        result = list(db["WeatherStation"].aggregate([match_stage, group_stage]))

        if not result:
            return jsonify({"status": "Failed", "message": "No data found"}), 404

        stats = result[0]
        stats.pop("_id", None)

        return jsonify({"status": "Success", "data": stats}), 200

    except Exception as e:
        return jsonify({"status": "Failed", "message": str(e)}), 500
#####################################
#   Routing for your application    #
#####################################


@app.route('/api/file/get/<filename>', methods=['GET']) 
def get_images(filename):   
    '''Returns requested file from uploads folder'''
   
    if request.method == "GET":
        directory   = join( getcwd(), Config.UPLOADS_FOLDER) 
        filePath    = join( getcwd(), Config.UPLOADS_FOLDER, filename) 

        # RETURN FILE IF IT EXISTS IN FOLDER
        if exists(filePath):        
            return send_from_directory(directory, filename)
        
        # FILE DOES NOT EXIST
        return jsonify({"status":"file not found"}), 404


@app.route('/api/file/upload',methods=["POST"])  
def upload():
    '''Saves a file to the uploads folder'''
    
    if request.method == "POST": 
        file     = request.files['file']
        filename = secure_filename(file.filename)
        file.save(join(getcwd(),Config.UPLOADS_FOLDER , filename))
        return jsonify({"status":"File upload successful", "filename":f"{filename}" })

 


###############################################################
# The functions below should be applicable to all Flask apps. #
###############################################################


@app.after_request
def after_request(response):
    """
    Add headers to both force latest IE rendering engine or Chrome Frame,
    and also tell the browser not to cache the rendered page. If we wanted
    to we could change max-age to 600 seconds which would be 10 minutes.
    """
    response.headers["Access-Control-Allow-Origin"] = "*"
    response.headers["Access-Control-Allow-Headers"] = "Content-Type,Authorization"
    response.headers["Access-Control-Allow-Methods"] = "GET,PUT,POST,DELETE,OPTIONS"

    response.headers['X-UA-Compatible'] = 'IE=Edge,chrome=1'
    response.headers['Cache-Control'] = 'public, max-age=0'
    return response

@app.errorhandler(404)
def page_not_found(error):
    """Custom 404 page."""    
    return jsonify({"status": 404}), 404

