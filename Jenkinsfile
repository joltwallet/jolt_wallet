pipeline {
    agent any
    environment {
        SSH_KEY = credentials('github-push')
    }
    stages {
        stage('Build Firmware') {
            when { tag "v*" }
            steps {
                // Save SSH Key to local dir
                withCredentials([sshUserPrivateKey(credentialsId: 'github-push', keyFileVariable: 'keyfile')]) {
                  sh 'export SSH_KEY=${keyfile}'
                }

                // Run Docker Container
                withDockerContainer(image: 'joltwallet/jolt_firmware:latest', args: '-v ~/.ssh/id_rsa:/home/jolt/.ssh/id_rsa'){}
            }
        }
        stage('TEST - Build Firmware') {
            steps {
                script {
                    docker.withRegistry('https://index.docker.io/v1/', 'docker-hub-credentials') {
                        sh 'docker pull "joltwallet/jolt_firmware:latest"'
                        echo "$SSH_KEY"
                        withCredentials([sshUserPrivateKey(credentialsId: 'github-push', keyFileVariable: 'SSH_KEY', passphraseVariable: '', usernameVariable: '')]) {
                            sh 'docker run -e SSH_KEY joltwallet/jolt_firmware:latest'
                        }
                    }
                }
            }
        }
    }
}
