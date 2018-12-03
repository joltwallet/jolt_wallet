import requests

get_work = '{ "action" : "block_count"}'
#r = requests.post('http://yapraiwallet.space:5523/api', data=get_work)
r = requests.post('http://45.55.67.202:8888', data=get_work)
print(r.text)
