from pymongo import MongoClient
from bson.objectid import ObjectId

class AnimalShelter(object):
    # Constructor: Initializes the database connection
    def __init__(self, user, password, host, port, db, col):
        """
        Initialize the AnimalShelter object with MongoDB connection.
        :param user: MongoDB username
        :param password: MongoDB password
        :param host: MongoDB host
        :param port: MongoDB port
        :param db: Database name
        :param col: Collection name
        """
        # Connect to MongoDB using credentials (secure coding: avoid hardcoding in production)
        self.client = MongoClient(f'mongodb://{user}:{password}@{host}:{port}')
        
        # Access the specified database and collection
        self.database = self.client[db]
        self.collection = self.database[col]
        
    # Create method: Inserts a new document into the collection
    def create(self, data):
        """
        Insert a document into the MongoDB collection.
        :param data: Dictionary containing the document fields.
        :return: Boolean indicating if insert was successful.
        """
        if data and isinstance(data, dict):  # Ensure data is a dictionary and not empty
            result = self.collection.insert_one(data)  # Insert document
            return result.inserted_id is not None  # Return True if insert was successful
        else:
            raise ValueError("Invalid or empty data provided. Must be a non-empty dictionary.")

    # Read method: Retrieves documents based on a query
    def read(self, query):
        """
        Retrieve documents from the MongoDB collection based on a query.
        :param query: Dictionary specifying the query criteria.
        :return: List of matching documents.
        """
        if query and isinstance(query, dict):  # Validate query format
            cursor = self.collection.find(query)
            return [document for document in cursor]  # Return matching documents as a list
        else:
            raise ValueError("Invalid or empty query provided. Must be a non-empty dictionary.")

# Main execution block
if __name__ == "__main__":
    # MongoDB connection parameters (for demonstration only; do not hardcode in production)
    USER = 'aacuser'
    PASS = '1997Ram!'
    HOST = 'nv-desktop-services-.apporto.com'
    PORT = 32215
    DB = 'aac'
    COL = 'animals'
    
    # Create an instance of the AnimalShelter class
    animal_shelter = AnimalShelter(USER, PASS, HOST, PORT, DB, COL)
    
    # Example document to insert
    data_to_insert = {
        'name': 'Murph',
        'animal': 'dog',
        'age': 1,
        'breed': 'German Shepherd'
    }
    
    # Insert the document into MongoDB
    insert_result = animal_shelter.create(data_to_insert)
    print(f"Insert Result: {insert_result}")
    
    # Query criteria to search for the inserted document
    query_criteria = {'name': 'Murph'}
    read_result = animal_shelter.read(query_criteria)
    print(f"Read Result: {read_result}")
