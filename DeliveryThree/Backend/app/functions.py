 #!/usr/bin/python3


#################################################################################################################################################
#                                                    CLASSES CONTAINING ALL THE APP FUNCTIONS                                                                                                    #
#################################################################################################################################################


class DB:

    def __init__(self,Config):

        from math import floor
        from os import getcwd
        from os.path import join
        from json import loads, dumps, dump
        from datetime import timedelta, datetime, timezone 
        from pymongo import MongoClient , errors, ReturnDocument
        from urllib import parse
        from urllib.request import  urlopen 
        from bson.objectid import ObjectId  
       
      
        self.Config                         = Config
        self.getcwd                         = getcwd
        self.join                           = join 
        self.floor                      	= floor 
        self.loads                      	= loads
        self.dumps                      	= dumps
        self.dump                       	= dump  
        self.datetime                       = datetime
        self.ObjectId                       = ObjectId 
        self.server			                = Config.DB_SERVER
        self.port			                = Config.DB_PORT
        self.username                   	= parse.quote_plus(Config.DB_USERNAME)
        self.password                   	= parse.quote_plus(Config.DB_PASSWORD)
        self.remoteMongo                	= MongoClient
        self.ReturnDocument                 = ReturnDocument
        self.PyMongoError               	= errors.PyMongoError
        self.BulkWriteError             	= errors.BulkWriteError  
        self.tls                            = False # MUST SET TO TRUE IN PRODUCTION


    def __del__(self):
            # Delete class instance to free resources
            pass
 

    def get_db(self):
        try:
            uri = f"mongodb://{self.username}:{self.password}@{self.server}:{self.port}/"
            client = self.remoteMongo(uri, tls=self.tls)
            return client["ELET2415"]
        except self.PyMongoError as e:
            print(f"Database connection error: {e}")
            return None
        
    
    ####################
    # DELIVERY FUNCTIONS  #
    ####################
    
    def add_update(self, data):
        try:
            db = self.get_db()
            if db is None:
                return {"status": "Failed", "message": "Database connection failed"}

            result = db["WeatherStation"].insert_one(data)

            return {
                "status": "Success",
                "message": "Document inserted successfully",
                "inserted_id": str(result.inserted_id)
            }

        except self.PyMongoError as e:
            return {
                "status": "Failed",
                "message": str(e)
            }


   



def main():
    from config import Config
    from time import time, ctime, sleep
    from math import floor
    from datetime import datetime, timedelta
    one = DB(Config)
 
 
    start = time() 
    end = time()
    print(f"completed in: {end - start} seconds")
    
if __name__ == '__main__':
    main()


    