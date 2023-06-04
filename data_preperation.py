import pandas as pd
import psycopg2

#Enables user to specify username via prompt
username = input("Please enter your username: ")
print("Username st to: " + username)

#Read textfile with sensor data 
data = pd.read_csv('SIMPLE.txt', sep=",", header=None, index_col=False)
#Specify column names in dataframe 
data.columns = ["Uhrzeit", "Temperatur", "Luftfeuchtigkeit", "Bewegung innen", "Bewegung außen", "Gewicht", "LED Status"]
print("File was read successfully")

#Fill in your credentials for establishing database connection in empty strings
ENDPOINT= ""
PORT= ""
USER= ""
REGION= ""
DBNAME= ""
PASSWORD = ""

#Establish database connection with given credentials
try:
    engine = psycopg2.connect(host=ENDPOINT, port=PORT, database=DBNAME, user=USER, password=PASSWORD)
    cursor = engine.cursor()
    print("Got connection")
except Exception as e:
    print("Database connection failed due to {}".format(e))  

#Data conversion from unix to datetime and add username column
data["Benutzer"] = username
data['Uhrzeit'] = pd.to_datetime(data['Uhrzeit'], unit = 's')

#SQL statement to create new database table according to schema
create_statement = "CREATE table if not exists Igeldaten (Uhrzeit varchar(255), Temperatur float, Luftfeuchtigkeit float, Innen boolean, Außen boolean, Gewicht float, LED boolean, Benutzer varchar(255));"

#Create new database if not exists already
cursor.execute(create_statement)
engine.commit()

#Iterate through entries of the dataframe and add new data point to database according to specified schema
for index, row in data.iterrows():
    cursor.execute("""INSERT into Igeldaten(Uhrzeit, Temperatur, Luftfeuchtigkeit, Innen, Außen, Gewicht, LED, Benutzer) values('%s','%s','%s','%s','%s','%s','%s','%s');""" %(str(data["Uhrzeit"][index]), data["Temperatur"][index], data["Luftfeuchtigkeit"][index], data["Bewegung innen"][index], data["Bewegung außen"][index], data["Gewicht"][index], data["LED Status"][index], data["Benutzer"][index]))
engine.commit()

print("Data upload to RDS successfull")

