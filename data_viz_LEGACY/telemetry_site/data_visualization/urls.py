from django.urls import path, include
from rest_framework import routers
from . import views

router = routers.DefaultRouter()
router.register('nodes', views.NodeViewSet)

urlpatterns = [ 
    path('api/', include(router.urls)), 
]