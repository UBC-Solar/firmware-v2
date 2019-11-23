'''
    A quick script to send data as post requests
'''

import requests 

url = "http://localhost:8000/data_viz/api/nodes/"
data1 = "CellC - 680mA DischCurr.xlsx"

### Read in data and post ### 


data = { 
    "voltage": 21.0, 
    "current": 1.3, 
    "capacity": 253
}

r = requests.post(url, data)
