import pandas as pd
from datetime import date
import smtplib
from email.mime.text import MIMEText



def send_email(myemail,mypswd,name,amount,day,category,receiver):
    
    body=f"""
    Dear {name}
        Your account with {amount}$ ends on {day}

        Please pay up!
        """
    msg=MIMEText(body,"plain","utf-8")
    msg["Subject"]=f"Payment Reminder-{category}"
    msg["From"]=myemail
    msg["To"]=receiver

    with smtplib.SMTP_SSL("smtp.gmail.com",465) as server:
        server.login(myemail,mypswd)
        server.send_message(msg)



today=date.today()

df=pd.read_csv("datafile.csv")


df["Ημερομηνία Λήξης του Λογαριασμού"]=pd.to_datetime(df["Ημερομηνία Λήξης του Λογαριασμού"]).dt.date

df["difference"]=(pd.to_datetime(df["Ημερομηνία Λήξης του Λογαριασμού"])-pd.Timestamp(today)).dt.days

myemail = "youremail@gmail.com"
mypswd = "your_app_password"


for index,row in df.iterrows():
    if row["difference"]<=3 and row["difference"]>=0 and row["Κατάσταση Ειδοποίησης"]!="Ειδοποίηση - Στάλθηκε":
        send_email(
            myemail, mypswd,
            row["Όνομα Χρήστη"],
            row["Ποσό Πληρωμής"],
            row["Ημερομηνία Λήξης του Λογαριασμού"],
            row["Κατηγορία Λογαριασμού"],
            row["Email Χρήστη"]
        )

        
        df.loc[index,"Κατάσταση Ειδοποίησης"]="Ειδοποίηση - Στάλθηκε"

df.to_csv("datafile.csv", index=False)