import requests


def upload(image_path: str) -> str:
    files = {'image': open(image_path, 'rb')}
    upload_response = requests.post('https://pz.al/api/upload', files=files)
    assert upload_response.status_code == requests.codes['ok']

    res_json = upload_response.json()
    assert res_json['code'] == 200

    url = res_json['data']['url']
    # print(url)
    return url
