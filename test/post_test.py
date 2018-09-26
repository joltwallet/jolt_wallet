import requests

get_work = '{ "action" : "block_count"}'
r = requests.post('http://yapraiwallet.space:5523/api', data=get_work)
print(r.text)
