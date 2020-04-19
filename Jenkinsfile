pipeline {
    agent any
    stages {
        stage('Build Firmware') {
            when { tag "v*" }
            steps {
                // Save SSH Key to local dir
                withCredentials([sshUserPrivateKey(credentialsId: 'github-push', keyFileVariable: 'keyfile')]) {
                  sh 'mkdir -p ~/.ssh && cp ${keyfile} ~/.ssh/id_rsa'
                }

                // Run Docker Container
                withDockerContainer(image: 'joltwallet/jolt_firmware:latest', args: '-v ~/.ssh/id_rsa:/home/jolt/.ssh/id_rsa'){}
            }
        }
        stage('TEST - Build Firmware') {
            steps {
                script {
                    docker.withRegistry('https://index.docker.io/v1/', 'docker-hub-credentials') {
                        sh 'id && ls -lha ~/.ssh'
                        sh 'docker pull "joltwallet/jolt_firmware:latest"'
                        sh 'docker run -v /home/nobody/.ssh/id_rsa:/home/jolt/.ssh/ joltwallet/jolt_firmware:latest' 
                    }
                }
            }
        }
    }
}
