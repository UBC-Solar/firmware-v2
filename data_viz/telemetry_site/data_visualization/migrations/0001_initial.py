# Generated by Django 2.0.6 on 2018-06-09 19:13

from django.db import migrations, models


class Migration(migrations.Migration):

    initial = True

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Node',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('timestamp', models.DateTimeField(auto_now=True)),
                ('voltage', models.DecimalField(decimal_places=5, max_digits=10)),
                ('current', models.DecimalField(decimal_places=5, max_digits=10)),
                ('capacity', models.DecimalField(decimal_places=5, max_digits=10)),
            ],
        ),
    ]