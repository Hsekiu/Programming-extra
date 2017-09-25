#Library packages
import os
import re
import sys
import json
import os.path
import boto.dynamodb2
#Installed packages
import boto.sqs
import boto.sqs.message
from boto.dynamodb2.fields import HashKey
from boto.dynamodb2.table import Table
from bottle import route, run, request, response, abort, default_app
from boto.exception import DynamoDBResponseError, JSONResponseError
def createEntry(request, database, PORT):
    '''
    #Query matching, empty list condition no implemented
    QUERY_PATTERN = "^id=[0-9]+&name=[a-zA-Z_]+&activities=([a-zA-Z_],?)+$"
    query_pattern = re.compile(QUERY_PATTERN)
    if not query_pattern.match(request.query_string):
        abort(400, "Query string does not match pattern '{0}'".format(QUERY_PATTERN))
    '''
    uid = str(request.query.id)
    name = request.query.name
    same = False
    if(len(request.query.activities) != 0):
        activities = (request.query.activities).split(',')
    else:
        activities = []
    #Check database for same id's and make sure that name and activites
    #are diffrent between the two.
    try:
        data = database.get_item(id=uid)
        #print "two id's are the same"
        d_name = data['name']
        d_act = data['activities']
        if(d_name != name or d_act != activities):
            #print "name or activties are the diffrent"
            response.status = 400
        else:
            #print "same item added"
            same = True
            response.status = 201
    #New id is unique
    except:
        response.status = 201
    print response.status
    try:
        if(response.status == "201 Created" and not(same)):
            database.put_item(data = {
                'id': uid,
                'name': name,
                'activities': activities,
            })
    except:
        #Ignore error same item added should return 201
        print "database error"
    #System API response sucess 201.
    if(response.status == "201 Created"):
        body = {
                'data' : {
                    'type' : 'person',
                    'id' : uid,
                    'links' : {
                        'self' : ('http://localhost:' + str(PORT) + '/retrieve?id=' + uid)   
                    }
                }
            }
    #System API response failure 400.
    else:
        body = {
            "errors": [{
                "id_exists": {
                        "status": "400",
                        "title": "id already exists",
                        "detail": {
                            "name": d_name,
                            "activities": d_act
                        }
                }
            }]
    }
    return json.dumps(body) + "\n"