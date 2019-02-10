import requests

get_work = '{ "action" : "block_count"}'
r = requests.post('https://yapraiwallet.space/quake/api', data=get_work)
#r = requests.post('https://yapraiwallet.space:5523/api', data=get_work)
print(r.text)
