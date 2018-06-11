from django.db import models

# Create your models here.
class Node(models.Model):  
    timestamp = models.DateTimeField(auto_now=True)
    voltage = models.DecimalField(max_digits=10, decimal_places=5)
    current = models.DecimalField(max_digits=10, decimal_places=5)
    capacity = models.DecimalField(max_digits=10, decimal_places=5) 
