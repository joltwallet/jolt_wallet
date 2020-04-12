import firebase_admin
from firebase_admin import credentials
from firebase_admin import storage
from os import environ
from os import path

def check_environment_variables:
	if environ.get('GOOGLE_APPLICATION_CREDENTIALS') is not None:
		if environ.get('UNCOMPRESSED_BINARY') is not None:
			if environ.get('COMPRESSED_BINARY') is not None:
				return True
			else:
				print('COMPRESSED_BINARY is not set!')
				return False
		else:
			print('UNCOMPRESSED_BINARY is not set!')
			return False
	else:
		print('GOOGLE_APPLICATION_CREDENTIALS is not set!')
		return False


def upload_binaries:
	if check_environment_variables():

		uncompressed_binary = environ.get('UNCOMPRESSED_BINARY')
		compressed_binary = environ.get('COMPRESSED_BINARY')
		
		uncompressed_binary_name = path.splitext("uncompressed_binary")[0]

		compressed_binary_name = path.splitext("compressed_binary")[0]

		joltwallet_bucket = firebase_admin.initialize_app({
		    'storageBucket': 'joltwallet-dev.appspot.com'
		})

		bucket = storage.bucket()

		uncompressed_binary_path = 'firmware/binaries/uncompressed/' + uncompressed_binary_name

		compressed_binary_path = 'firmware/binaries/compressed/' + compressed_binary_name

		# Create Blob objects on Firebase
		blob = bucket.blob(uncompressed_binary_path)
		blob = bucket.blob(compressed_binary_path)

		# Upload the Binary files to Firebase
		blob.upload_from_filename(uncompressed_binary)
		blob.upload_from_filename(compressed_binary)

		uncompressed_binary_url = bucket.name + uncompressed_binary_path

		compressed_binary_url = bucket.name + compressed_binary_path

		os.environ['UNCOMPRESSED_BINARY_URL'] = uncompressed_binary_url
		os.environ['COMPRESSED_BINARY_URL'] = compressed_binary_url

	else:
		print('Environment Variables are not properly set! Aborting.')

upload_binaries()

