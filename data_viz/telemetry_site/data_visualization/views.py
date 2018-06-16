from django.shortcuts import render
from rest_framework import viewsets 
from rest_framework import routers 

from .serializers import NodeSerializer
from .models import Node 

def index(request): 
    return render(request, 'templates/index.html')

class NodeViewSet(viewsets.ModelViewSet): 
    """
    API endpoint for node data 
    """
    serializer_class = NodeSerializer
    queryset = Node.objects.all().order_by('-timestamp')