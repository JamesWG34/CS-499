from pymongo import MongoClient 
from bson.objectid import ObjectId

class AnimalShelter(object):
    def _init_(self, user, password, host, port, db, col):
        self.client = MongoClient(f'mongodb://{user}:{password}@{host}:{port}')
        self.database = self.client[db]
        self.collection = self.database[col]
        
        
    def create(self, data): 
        if data is not None: 
            results = self.collection.insert_one(data)
            return True if result.inserted_id else False
        else:
            raise Exception("Nothing to save because data parameter is empty")
           
    def read(self, query):
        cursor = self.collection.find(query)
        result = [document for document in cursor]
        return result

if_name_ == "_main_":
    USER = 'aacuser'
    PASS = '1997Ram!'
    HOST = 'nv-desktop-services-.apporto.com'
    PORT = 32215
    DB = 'aac'
    COL = 'animals'
    
    animal_shelter = AnimalShelter(USER, PASS, HOST, PORT, DB, COL)
    
    data_to_insert = {
        'name': 'Murph',
        'animal': 'dog',
        'age' : 1,
        'breed': 'German Shepard'
    }
    
    insert_result = animal_shelter.create(data_to_insert)
    print(f"Insert Result: {insert_result}")
    
    query_criteria = {'name': 'Murph'}
    read_result = animal_shelter.read(query_criteria)
    print(f"Read Result: {read_result}")