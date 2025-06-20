from flask import Flask,render_template,request,url_for,redirect,flash

app = Flask(__name__)

@app.route("/")
# Direccionamiento al index o home
def Home_web(): 
    return render_template('index.html')