Purpose
===
This folder houses the Django project for our REST API to send and store data. For now, this project is intended to provide some simple 'pseudo realtime' data visualization for the purposes of presenting at the Alumni Event on June 27th, 2018. 

Files and folder structure 
=== 
1. The core of the Django project is in `telemetry_site > data_visualization`.
2. `send_data.py` is a simple POST request script to our API located at `http://localhost:8000/data_viz/api/nodes`. 

Usage
=== 
1. Install the dependencies with `pip install -r requirements.txt`. 
2. Run the server locally with `python manage.py runserver`. 
3. Navigate to `http://localhost/data_viz/api/nodes` to view the current data. 
4. Run the `send_data.py` script to send one packet of data, refresh the site to see the change. 

TODO
===
**send_data.py**
1. Read in data from `CellC - 680mA DischCurr.xlsx`. 
2. Send the data in a loop. 

**telemetry_site/data_visualization/templates/**
1. Add frontend visualizations (HTML/CSS files + JS script to periodically poll the API to update the visualization)