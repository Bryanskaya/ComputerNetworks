from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email.utils import formatdate
from email import encoders

import smtplib
import time


def input_data(data):
    print("--- Input data for sending mail ---")
    data["toUser"] = input("To: ")
    data["fromUser"] = input("From: ")
    data["password"] = input("Password: ")
    data["interval"] = input("Time interval: ")

    print("\n--- Mail ---")
    data["title"] = input("Title: ")
    data["body"] = input("Body: ")
    data["filename"] = input("Filename (optional): ")


def create_msg(data):
    msg = MIMEMultipart()
    msg["From"] = data['fromUser']
    msg["To"] = data['toUser']
    msg["Date"] = formatdate(localtime=True)
    msg["Subject"] = data['title']
    msg.attach(MIMEText(data['body']))

    attachment = MIMEBase('application', "octet-stream")

    if data["filename"] != '':
        try:
            with open(data["filename"], "rb") as f:
                tempData = f.read()

            attachment.set_payload(tempData)
            encoders.encode_base64(attachment)
            attachment.add_header('Content-Disposition', 'attachment; filename="%s"' % data["filename"])

            msg.attach(attachment)
        except Exception:
            print("Error in open() file %s" % data["filename"])

    return msg


def send_mail(msg, data):
    smtp_ssl_host = 'smtp.yandex.ru'
    smtp_ssl_port = 465

    while True:
        try:
            server_ssl = smtplib.SMTP_SSL(smtp_ssl_host, smtp_ssl_port)
            server_ssl.login(data["fromUser"], data["password"])
            server_ssl.sendmail(data["fromUser"], data["toUser"], msg.as_string())
            server_ssl.quit()

            print('Email was send')
        except Exception as exp:
            print("Error in sending mail %s" % exp)

        time.sleep(int(data["interval"]))


if __name__ == "__main__":
    data = {}

    input_data(data)
    msg = create_msg(data)
    send_mail(msg, data)





